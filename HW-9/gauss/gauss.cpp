#include "gauss.h"
#include "barrier.h"
#include "threads_guard.h"
#include <thread>
#include <algorithm>
#include <cmath>

const double EPS = 10e-9;

size_t get_start_index_for_cyclic_partition(size_t val, size_t index, size_t num_of_threads)
{
	if (index < (val % num_of_threads))
	{
		return val - (val % num_of_threads) + num_of_threads + index;
	}
	return val - (val % num_of_threads) + index;
}

bool gauss(const std::vector<std::vector<double> >& input, std::vector<double>& ans, size_t num_of_threads)
{
	std::vector<std::vector<double> > a = input;
	cyclic_barrier barrier(num_of_threads);
	size_t n = a.size();
	std::vector<size_t> parallel_max_row(num_of_threads);
	std::vector<bool> ret_val(num_of_threads, true);
	std::vector<std::thread> threads;
	threads_guard guard(threads);
	size_t max_row;
	size_t row = 0;
	std::vector<int> base(n, -1);
	for (size_t i = 0; i < num_of_threads; ++i)
	{
		threads.emplace_back([&, i]
		{
			for (size_t column = 0; column < n; ++column)
			{
				while (column < n)
				{
					parallel_max_row[i] = row;
					for (size_t cur_row = get_start_index_for_cyclic_partition(row, i, num_of_threads); cur_row < n; cur_row += num_of_threads)
					{
						if (std::abs(a[parallel_max_row[i]][column]) < std::abs(a[cur_row][column]))
						{
							parallel_max_row[i] = cur_row;
						}
					}
					barrier.enter();
					if (i == 0)
					{
						if (column == n)
						{
							column++;
							break;
						}
						max_row = parallel_max_row[0];
						for (size_t j = 1; j < num_of_threads; ++j)
						{
							if (a[max_row][column] < a[parallel_max_row[j]][column])
							{
								max_row = parallel_max_row[j];
							}
						}
					}
					barrier.enter();
					if (std::abs(a[max_row][column++]) >= EPS)
					{
						break;
					}
					if (column == n)
					{
						column++;
						break;
					}
				}
				column--;
				if (column == n)
				{
					for (size_t cur_row = get_start_index_for_cyclic_partition(row, i, num_of_threads); cur_row < n; cur_row += num_of_threads)
					{
						if (std::abs(a[cur_row][n]) >= EPS)
						{
							ret_val[i] = false;
							return;
						}
					}
					break;
				}
				if (max_row != row)
				{
					for (size_t index = get_start_index_for_cyclic_partition(column, i, num_of_threads); index <= n; index += num_of_threads)
					{
						std::swap(a[max_row][index], a[row][index]);
					}
				}
				barrier.enter();
				for (size_t cur_row = i; cur_row < n; cur_row += num_of_threads)
				{
					if (cur_row == row)
					{
						continue;
					}
					double coef = a[cur_row][column] / a[row][column];
					for (size_t index = column; index <= n; ++index)
					{
						a[cur_row][index] -= a[row][index] * coef;
					}
				}
				barrier.enter();
				for (size_t index = get_start_index_for_cyclic_partition(column + 1, i, num_of_threads); index <= n; index += num_of_threads)
				{
					a[row][index] /= a[row][column];
				}
				barrier.enter();
				if (i == 0)
				{
					base[column] = row;
					a[row][column] = 1.0;
					++row;
				}
				barrier.enter();
			}
			for (size_t column = i; column < n; column += num_of_threads)
			{
				if (base[column] != -1)
				{
					ans[column] = a[base[column]][n];
				}
			}
		});
	}
	for (auto& it : threads)
	{
		it.join();
	}
	for (auto it : ret_val)
	{
		if (!it)
		{
			return false;
		}
	}
	return true;
}
