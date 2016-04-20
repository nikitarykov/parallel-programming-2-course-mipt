#ifndef FFT_H
#define FFT_H

#define _USE_MATH_DEFINES
#include "cmath"
#include <iostream>
#include <vector>
#include <complex>
#include <thread>
#include "barrier.h"
#include "threads_guard.h"
#include <algorithm>

size_t get_pow_of_2(size_t in)
{
	size_t res = 1;
	while (res < in)
	{
		res *= 2;
	}
	return res;
}

size_t reverse(size_t val, size_t pos) 
{
	size_t res = 0;
	for (size_t i = 0; i < pos; ++i)
	{
		if (val & (1 << i))
		{
			res |= 1 << (pos - 1 - i);
		}
	}
	return res;
}

void fft(size_t i, size_t n, std::vector<std::complex<double>>& FA, cyclic_barrier& barrier, bool is_ifft, size_t num_of_threads)
{
	size_t log_n = 0;
	while ((1 << ++log_n) < n);
	for (size_t j = i; j < n; j += num_of_threads)
	{
		size_t rev = reverse(j, log_n);
		if (j < rev)
		{
			std::swap(FA[j], FA[rev]);
		}
	}
	barrier.enter();
	size_t step = 1;
	while (step < (n / num_of_threads))
	{
		step *= 2;
		double arg = 2.0 * M_PI * (is_ifft ? -1.0 : 1.0) / (double)step;
		std::complex<double> w_step(cos(arg), sin(arg));
		for (size_t k = i * (n / num_of_threads); k < (i + 1) * (n / num_of_threads); k += step)
		{
			std::complex<double> w(1.0);
			for (size_t p = 0; p < step / 2; ++p)
			{
				std::complex<double> y = FA[k + p];
				std::complex<double> v = w * FA[k + p + step / 2];
				FA[k + p] = y + v;
				FA[k + p + step / 2] = y - v;
				w *= w_step;
			}
		}
	}
	barrier.enter();
	while (step < n)
	{
		step *= 2;
		double arg = 2.0 * M_PI * num_of_threads * (is_ifft ? -1.0 : 1.0) / (double)step;
		std::complex<double> w_step(cos(arg), sin(arg));
		for (size_t k = i; k + step / 2 < n; k += step)
		{
			double arg0 = 2.0 * M_PI * i * (is_ifft ? -1.0 : 1.0) / (double)step;
			std::complex<double> w(cos(arg0), sin(arg0));
			for (size_t p = 0; p < step / 2 && k + p + step / 2 < n; p += num_of_threads)
			{
				std::complex<double> y = FA[k + p];
				std::complex<double> v = w * FA[k + p + step / 2];
				FA[k + p] = y + v;
				FA[k + p + step / 2] = y - v;
				w *= w_step;
			}
		}
	}
}
void multiply_polynomials(const std::vector<int>& A, const std::vector<int>& B, std::vector<int>& C, size_t num_of_threads = std::thread::hardware_concurrency())
{
	size_t n = get_pow_of_2(A.size());
	size_t m = get_pow_of_2(B.size());
	std::vector<std::complex<double>> FA(A.begin(), A.end());
	std::vector<std::complex<double>> FB(B.begin(), B.end());
	n = 2 * std::max(n, m);
	FA.resize(n);
	FB.resize(n);
	std::vector<std::thread> threads;
	if ((num_of_threads & (num_of_threads - 1)) != 0)
	{
		size_t new_n = 1;
		while (new_n < num_of_threads)
		{
			new_n *= 2;
		}
		num_of_threads = new_n;
	}
	while (num_of_threads * num_of_threads > n)
	{
		num_of_threads /= 2;
	}
	threads_guard guard(threads);
	cyclic_barrier barrier(num_of_threads);
	for (size_t i = 0; i < num_of_threads; ++i)
	{
		threads.emplace_back([&, i]
		{
			fft(i, n, FA, barrier, false, num_of_threads);
			fft(i, n, FB, barrier, false, num_of_threads);
			barrier.enter();
			for (size_t j = i; j < n; j += num_of_threads)
			{
				FA[j] *= FB[j];
			}
			barrier.enter();
			fft(i, n, FA, barrier, true, num_of_threads);
		});
	}
	for (auto& it : threads)
	{
		it.join();
	}
	C.resize(A.size() + B.size() - 1);
	for (size_t i = 0; i < C.size(); ++i)
	{
		C[i] = (int)(FA[i].real() / (double)n + 0.5);
	}
}

#endif