#ifndef CYCLIC_BARRIER_H
#define CYCLIC_BARRIER_H

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

class cyclic_barrier
{
	size_t num_of_threads;
	std::atomic<size_t> count_of_waiting;
	std::atomic<size_t> step;

public:
	cyclic_barrier(const size_t _num_of_threads) : num_of_threads(_num_of_threads), count_of_waiting(0), step(0) {}

	cyclic_barrier(const cyclic_barrier&) = delete;
	cyclic_barrier& operator=(const cyclic_barrier&) = delete;

	void enter()
	{
		size_t cur_step = step.load();
		if (count_of_waiting.fetch_add(1) >= num_of_threads - 1)
		{
			count_of_waiting.store(0);
			step.fetch_add(1);
		}
		else
		{
			while (step.load() == cur_step)
			{
				std::this_thread::yield();
			}
		}
	}
};

#endif