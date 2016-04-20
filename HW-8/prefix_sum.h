#ifndef PREFIX_SUM_H
#define PREFIX_SUM_H

#include <vector>
#include <thread>
#include <algorithm>
#include "cyclic_barrier.h"

template <typename T, class Add>
void parallel_scan(const std::vector<T>& data, Add add, std::vector<T>& prefix_sums, size_t num_threads)
{
	std::vector<std::thread> threads;
	std::vector<T> buf(data.size());
	cyclic_barrier barrier(num_threads);
	size_t last_iter = 0;
	size_t block_size = data.size() % num_threads == 0 ? data.size() / num_threads : data.size() / num_threads + 1;
	prefix_sums = data;
	for (size_t i = 0; i < num_threads; ++i)
		threads.emplace_back(
		[&, i]()
		{
			std::vector<T>* cur_buf = &prefix_sums;
			std::vector<T>* next_buf = &buf;
			for (size_t iter = 0, step = 1; step < data.size(); ++iter, step *= 2)
			{
				for (size_t j = i * block_size; j < std::min(data.size(), block_size * (i + 1)); ++j)
				{
					if (j < step)
					{
						(*next_buf)[j] = (*cur_buf)[j];
					}
					else
					{
						(*next_buf)[j] = add((*cur_buf)[j], (*cur_buf)[j - step]);
					}
				}
				swap(cur_buf, next_buf);
				if (i == 0)
				{
					last_iter = iter;
				}
				barrier.enter();
			}
		});
	for (auto& t : threads)
	{
		t.join();
	}
	if (last_iter % 2 == 0)
	{
		prefix_sums.swap(buf);
	}
}

#endif