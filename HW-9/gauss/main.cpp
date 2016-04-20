#include <iostream>
#include <vector>
#include <cmath>
#include "gauss.h"
#include "steady_timer.h"

const double EPS = 1e-9;

size_t n;
int main() {
	freopen("input.txt", "r", stdin);
	freopen("output.txt", "w", stdout);

	std::cin >> n;
	std::vector<std::vector<double> > A(n), _A(n);
	std::vector<double> _b(n), x(n);
	for (size_t i = 0; i < n; ++i)
	{
		A[i].resize(n + 1);
		_A[i].resize(n);
	}
	for (size_t i = 0; i < n; ++i) 
	{
		for (size_t j = 0; j < n; ++j) 
		{
			std::cin >> A[i][j];
			_A[i][j] = A[i][j];
		}
	}

	// input vector b

	for (size_t j = 0; j < n; ++j) {
		std::cin >> A[j][n];
		_b[j] = A[j][n];
	}

	steady_timer timer;

	bool res = gauss(A, x);
	
	const double work_time = timer.seconds_elapsed();

	if (!res)
	{
		printf("no solution\n");
	}
	else
	{
		for (size_t i = 0; i < n; ++i) {
			printf("%14.8lf ", x[i]);
		}
		printf("\n");
		
		double err = 0;
		for (size_t i = 0; i < n; ++i) {
			double lhs = 0;
			for (size_t j = 0; j < n; ++j) {
				lhs += _A[i][j] * x[j];
			}
			double r = _b[i] - lhs;
			err += r * r;
		}

		printf("err = %.10lf\n", err);
	}

	printf("work time = %.5lf\n", work_time);

	return 0;
}