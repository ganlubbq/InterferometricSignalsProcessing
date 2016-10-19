#include <iostream>
#include <functional>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <random>

#include <Eigen/Dense>
//#include <opencv/cv.h>
//#include <opencv/highgui.h>

#include "SignalMaker.h"
#include "ExtendedKalmanFilterIS1D.h"
#include "StatePrinter.h"
#include "SignalAnalysis.h"
#include "TotalSearchTuner.h"
#include "GradientTuner.h"
#include "SymbolicTree.h"
#include "SymbolicRegression.h"

double** getLearningSignals(int sigCount, double *background, double *frequency, double E_min, double E_max, double sigma, double delta_z, const int N, std::default_random_engine &gen)
{
	double **signals = new double*[sigCount];
	for (int k = 0; k < sigCount; k++)
	{
		double startPhase = (double)(gen() % 100000000) / 100000000 * 2 * M_PI;
		double *phase = SignalMaker::phaseFromFrequency(frequency, startPhase, N, delta_z);
		double *noise = SignalMaker::normalDistribution(0, 10, N, gen);
		double *amplitude = SignalMaker::randomGaussianAmplitude(N, E_min, E_max, sigma, 6, gen);

		signals[k] = SignalMaker::createSignal1D(background, amplitude, phase, noise, N);

		delete[] phase;
		delete[] noise;
		delete[] amplitude;
	}
	return signals;
}

ExtendedKalmanFilterIS1D getTunedKalman_TotalSearch(double **signals, const int N, int sigCount, std::default_random_engine &gen)
{
	//Creation of EKF tuned by TotalSearch
	ExtendedKalmanFilterIS1DState minimal;
	ExtendedKalmanFilterIS1DState maximal;

	minimal.state = Eigen::Vector4d(0, 0, 0.00, 0);
	minimal.Rw <<
		0.1, 0, 0, 0,
		0, 0.15, 0, 0,
		0, 0, 0.001, 0,
		0, 0, 0, 0.002;
	minimal.R = minimal.Rw;
	minimal.Rn = 0.1;

	maximal.state = Eigen::Vector4d(255, 140, 0.158, 2 * M_PI);
	maximal.Rw <<
		0.1, 0, 0, 0,
		0, 0.15, 0, 0,
		0, 0, 0.001, 0,
		0, 0, 0, 0.002;
	maximal.R = maximal.Rw;
	maximal.Rn = 10;

	StatePrinter::console_print_full_Kalman_state(ExtendedKalmanFilterIS1DState());
	FilterTuning::TotalSearchTuner tuner(signals, N, sigCount, 10, gen, minimal, maximal);
	tuner.createStates();
	ExtendedKalmanFilterIS1DState tunedParameters = tuner.tune();
	StatePrinter::console_print_full_Kalman_state(tunedParameters);
	return ExtendedKalmanFilterIS1D(tunedParameters);
}

void estimate(ExtendedKalmanFilterIS1D &EKF, double *signal, Eigen::Vector4d *states, double *restoredSignal, int N)
{
	for (int i = 0; i < N; i++)
	{
		EKF.estimate(signal[i]);
		states[i] = EKF.getState();
		restoredSignal[i] = EKF.evaluateSignalValue();
	}
}

ExtendedKalmanFilterIS1D getTunedKalman_Gradient(ExtendedKalmanFilterIS1DState begin, ExtendedKalmanFilterIS1DState step, 
	double **signals, const int N, int sigCount, int iterationsCount)
{
	FilterTuning::GradientTuner tuner(signals, N, sigCount, iterationsCount, begin, step);
	ExtendedKalmanFilterIS1DState tunedParameters = tuner.tune();
	//StatePrinter::console_print_full_Kalman_state(tunedParameters);
	return ExtendedKalmanFilterIS1D(tunedParameters);
}


int main(int argc, char **argv)
{

	//Signals modeling
	const int N = 500;
	const double delta_z = 1;

	int sigCount = 20;	//learning signals count
	double E_min = 50;	//Max amplitude
	double E_max = 100;	//Max amplitude
	double sigma = 50;

	double background[N];
	double frequency[N];

	for (int i = 0; i < N; i++)
	{
		background[i] = 130;
		frequency[i] = 0.17985 - 0.0002*i;
	}

	//Learning signals
	std::default_random_engine gen((unsigned int)time(NULL));
	double **signals = getLearningSignals(sigCount, background, frequency, E_min, E_max, sigma, delta_z, N, gen);

	//Estimated signal
	int edges[3] = { 100, 200, 425 };
	double *amplitude = SignalMaker::fixedGaussianAmplitude(N, E_min, E_max, sigma, edges, 3);
	double *phase = SignalMaker::phaseFromFrequency(frequency, 0, N, delta_z);
	double *noise = SignalMaker::normalDistribution(0, 10, N, gen);
	double *signal = SignalMaker::createSignal1D(background, amplitude, phase, noise, N);
	StatePrinter::print_signal("out.txt", signal, N);
	StatePrinter::print_states("data.txt", background, amplitude, frequency, phase, N);

	//test arrays
	Eigen::Vector4d *states = new Eigen::Vector4d[N];
	double *restoredSignal = new double[N];

	//Without GD
	//Creation of EKF
	Eigen::Vector4d beginState(100, 70, 0.05, 0);
	Eigen::Matrix4d Rw;
	Rw << 0.1, 0, 0, 0,
		0, 0.15, 0, 0,
		0, 0, 0.005, 0,
		0, 0, 0, 0.002;
	double Rn = 0.5;
	ExtendedKalmanFilterIS1D EKF(beginState, Eigen::Matrix4d::Identity(), Rw, Rn);

	estimate(EKF, signal, states, restoredSignal, N);
	StatePrinter::print_states("EKFdata.txt", states, N);
	StatePrinter::print_Kalman_stdev("EKFdeviations.txt", states, signal, noise, background, amplitude, frequency, phase, restoredSignal, N);

	//With GD
	ExtendedKalmanFilterIS1DState begin;
	ExtendedKalmanFilterIS1DState step;

	begin.state = Eigen::Vector4d(100, 70, 0.05, 1);
	begin.Rw <<
		0.1, 0, 0, 0,
		0, 0.15, 0, 0,
		0, 0, 0.005, 0,
		0, 0, 0, 0.002;
	begin.R = Eigen::Matrix4d::Identity();
	begin.Rn = 5;

	step.state = Eigen::Vector4d(1, 1, 0.0001, 0.05);
	step.Rw <<
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0;
	step.R = step.Rw;
	step.Rn = 0.1;
	EKF = getTunedKalman_Gradient(begin, step, signals, N, sigCount, 5);
	estimate(EKF, signal, states, restoredSignal, N);

	StatePrinter::print_states("GDdata.txt", states, N);
	StatePrinter::print_Kalman_stdev("GDdeviations.txt", states, signal, noise, background, amplitude, frequency, phase, restoredSignal, N);

	//Super tests
	//1 only vector
	Eigen::Vector4d *deviations = new Eigen::Vector4d[100];
	Eigen::Vector4d *starts = new Eigen::Vector4d[100];

	begin.state = Eigen::Vector4d(100, 70, 0.05, 1);
	begin.Rw <<
		0.1, 0, 0, 0,
		0, 0.15, 0, 0,
		0, 0, 0.005, 0,
		0, 0, 0, 0.002;
	begin.R = Eigen::Matrix4d::Identity();
	begin.Rn = 5;

	step.state = Eigen::Vector4d(1, 1, 0.0001, 0.05);
	step.Rw <<
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0;
	step.R <<
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0;
	step.Rn = 0.1;

	for (int i = 5; i < 505; i+=5)
	{
		int k = i / 5 - 1;
		EKF = getTunedKalman_Gradient(begin, step, signals, N, sigCount, 5);
		begin = EKF.getFullState();
		starts[k] = begin.state;
		estimate(EKF, signal, states, restoredSignal, N);
		deviations[k] = SignalAnalysis::get_deviations(states, signal, noise, background, amplitude, frequency, phase, restoredSignal, N);
		
		std::stringstream str;
		str << "GD_1_" << i << ".txt\0";
		StatePrinter::print_states(str.str().c_str(), states, N);
	}
	StatePrinter::print_states("GD_1_deviations.txt", deviations, 100);
	StatePrinter::print_states("GD_1_starts.txt", starts, 100);

	//Memory release
	for (int i = 0; i < sigCount; i++)
	{
		delete[] signals[i];
	}
	delete[] signals;

	//Signal parameters
	delete[] amplitude;
	delete[] noise;
	delete[] phase;
	delete[] signal;
	delete[] restoredSignal;

	//States
	delete[] states;

	return 0;
}

////GD 2
//begin.state = Eigen::Vector4d(100, 70, 0.05, 1);
//begin.Rw <<
//0.1, 0, 0, 0,
//0, 0.15, 0, 0,
//0, 0, 0.005, 0,
//0, 0, 0, 0.002;
//begin.R = Eigen::Matrix4d::Identity();
//begin.Rn = 5;
//
//step.state = Eigen::Vector4d(1, 1, 0.0001, 0.05);
//step.Rw <<
//0.001, 0, 0, 0,
//0, 0.001, 0, 0,
//0, 0, 0.00001, 0,
//0, 0, 0, 0.0001;
//step.R <<
//0, 0, 0, 0,
//0, 0, 0, 0,
//0, 0, 0, 0,
//0, 0, 0, 0;
//step.Rn = 0.1;
//
//for (int i = 5; i < 505; i += 5)
//{
//	int k = i / 5 - 1;
//	EKF = getTunedKalman_Gradient(begin, step, signals, N, sigCount, 5);
//	begin = EKF.getFullState();
//	starts[k] = begin.state;
//	estimate(EKF, signal, states, restoredSignal, N);
//	deviations[k] = SignalAnalysis::get_deviations(states, signal, noise, background, amplitude, frequency, phase, restoredSignal, N);
//
//	std::stringstream str;
//	str << "GD_2_" << i << ".txt\0";
//	StatePrinter::print_states(str.str().c_str(), states, N);
//}
//StatePrinter::print_states("GD_2_deviations.txt", deviations, 100);
//StatePrinter::print_states("GD_2_starts.txt", starts, 100);