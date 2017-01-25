#include <fstream>
#include <iostream>
#include "StatePrinter.h"
#include "../SignalAnalysis/SignalAnalysis.h"

void StatePrinter::print_states(const char *filename, Eigen::Vector4d *states, int N)
{
	std::ofstream out(filename);
	for (int i = 0; i < N; i++)
	{
		out << states[i](0) << "\t" << states[i](1) << "\t" << states[i](2) << "\t" << states[i](3) << std::endl;
	}
	out.close();
}

void StatePrinter::print_states(const char *filename, double *background, double *amplitude, double *frequency, double *phase, int N)
{
	std::ofstream out(filename);
	for (int i = 0; i < N; i++)
	{
		out << background[i] << "\t" << amplitude[i] << "\t" << frequency[i] << "\t" << phase[i] << std::endl;
	}
	out.close();
}

void StatePrinter::print_signal(const char *filename, double *signal, int N)
{
	std::ofstream out(filename);
	for (int i = 0; i < N; i++)
	{
		out << signal[i] << std::endl;
	}
	out.close();
}

void StatePrinter::print_full_Kalman_states(const char *filename, ExtendedKalmanFilterIS1DState *states, int N)
{
	std::ofstream out(filename);
	for (int i = 0; i < N; i++)
	{
		out << states[i].state(0) << "\t" << states[i].state(1) << "\t" << states[i].state(2) << "\t" << states[i].state(3) << std::endl << std::endl;

		out << states[i].R(0, 0) << " " << states[i].R(0, 1) << " " << states[i].R(0, 2) << " " << states[i].R(0, 3) << std::endl;
		out << states[i].R(1, 0) << " " << states[i].R(1, 1) << " " << states[i].R(1, 2) << " " << states[i].R(1, 3) << std::endl;
		out << states[i].R(2, 0) << " " << states[i].R(2, 1) << " " << states[i].R(2, 2) << " " << states[i].R(2, 3) << std::endl;
		out << states[i].R(3, 0) << " " << states[i].R(3, 1) << " " << states[i].R(3, 2) << " " << states[i].R(3, 3) << std::endl << std::endl;

		out << states[i].Rn << std::endl << std::endl;

		out << states[i].Rw(0, 0) << " " << states[i].Rw(0, 1) << " " << states[i].Rw(0, 2) << " " << states[i].Rw(0, 3) << std::endl;
		out << states[i].Rw(1, 0) << " " << states[i].Rw(1, 1) << " " << states[i].Rw(1, 2) << " " << states[i].Rw(1, 3) << std::endl;
		out << states[i].Rw(2, 0) << " " << states[i].Rw(2, 1) << " " << states[i].Rw(2, 2) << " " << states[i].Rw(2, 3) << std::endl;
		out << states[i].Rw(3, 0) << " " << states[i].Rw(3, 1) << " " << states[i].Rw(3, 2) << " " << states[i].Rw(3, 3) << std::endl << std::endl;
	}
	out.close();
}

void StatePrinter::console_print_full_Kalman_state(ExtendedKalmanFilterIS1DState &state)
{
		std::cout << state.state(0) << "\t" << state.state(1) << "\t" << state.state(2) << "\t" << state.state(3) << std::endl << std::endl;

		std::cout << state.R(0, 0) << " " << state.R(0, 1) << " " << state.R(0, 2) << " " << state.R(0, 3) << std::endl;
		std::cout << state.R(1, 0) << " " << state.R(1, 1) << " " << state.R(1, 2) << " " << state.R(1, 3) << std::endl;
		std::cout << state.R(2, 0) << " " << state.R(2, 1) << " " << state.R(2, 2) << " " << state.R(2, 3) << std::endl;
		std::cout << state.R(3, 0) << " " << state.R(3, 1) << " " << state.R(3, 2) << " " << state.R(3, 3) << std::endl << std::endl;

		std::cout << state.Rn << std::endl << std::endl;

		std::cout << state.Rw(0, 0) << " " << state.Rw(0, 1) << " " << state.Rw(0, 2) << " " << state.Rw(0, 3) << std::endl;
		std::cout << state.Rw(1, 0) << " " << state.Rw(1, 1) << " " << state.Rw(1, 2) << " " << state.Rw(1, 3) << std::endl;
		std::cout << state.Rw(2, 0) << " " << state.Rw(2, 1) << " " << state.Rw(2, 2) << " " << state.Rw(2, 3) << std::endl;
		std::cout << state.Rw(3, 0) << " " << state.Rw(3, 1) << " " << state.Rw(3, 2) << " " << state.Rw(3, 3) << std::endl << std::endl;
}

void StatePrinter::print_Kalman_stdev(const char *filename, Eigen::Vector4d *states, double *signal, double *noise, 
	double *background, double *amplitude, double *frequency, double *phase, double *restoredSignal, int N)
{
	std::ofstream out(filename);
	double *tmp = new double[N];
	
	for (int i = 0; i < N; i++)
	{
		tmp[i] = states[i](0) - background[i];
	}
	out << SignalAnalysis::stdev(tmp, N) << std::endl;
	for (int i = 0; i < N; i++)
	{
		tmp[i] = states[i](1) - amplitude[i];
	}
	out << SignalAnalysis::stdev(tmp, N) << std::endl;
	for (int i = 0; i < N; i++)
	{
		tmp[i] = states[i](2) - frequency[i];
	}
	out << SignalAnalysis::stdev(tmp, N) << std::endl;
	for (int i = 0; i < N; i++)
	{
		tmp[i] = states[i](3) - phase[i];
	}
	out << SignalAnalysis::stdev(tmp, N) << std::endl;

	SignalAnalysis::diff(signal, restoredSignal, tmp, N);
	out << SignalAnalysis::snr(signal, tmp, N) << std::endl;
	
	delete[] tmp;
	out.close();
}

void StatePrinter::console_print_Kalman_stdev(Eigen::Vector4d *states, double *signal, double *noise,
	double *background, double *amplitude, double *frequency, double *phase, double *restoredSignal, int N)
{
	double *tmp = new double[N];
	std::cout << "Standard deviations:" << std::endl;
	for (int i = 0; i < N; i++)
	{
		tmp[i] = states[i](0) - background[i];
	}
	std::cout << "Background error:\t" << SignalAnalysis::stdev(tmp, N) << std::endl;
	for (int i = 0; i < N; i++)
	{
		tmp[i] = states[i](1) - amplitude[i];
	}
	std::cout << "Amplitude error:\t" << SignalAnalysis::stdev(tmp, N) << std::endl;
	for (int i = 0; i < N; i++)
	{
		tmp[i] = states[i](2) - frequency[i];
	}
	std::cout << "Frequency error:\t" << SignalAnalysis::stdev(tmp, N) << std::endl;
	for (int i = 0; i < N; i++)
	{
		tmp[i] = states[i](3) - phase[i];
	}
	std::cout << "Phase error:\t\t" << SignalAnalysis::stdev(tmp, N) << std::endl;

	SignalAnalysis::diff(signal, restoredSignal, tmp, N);
	std::cout << "Signal-to-noise ratio:\t" << SignalAnalysis::snr(signal, tmp, N) << std::endl;
	
	delete[] tmp;
}