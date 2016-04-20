#ifndef CYCLIC_BARRIER_H
#define CYCLIC_BARRIER_H

#include <iostream>
#include <mutex>
#include <condition_variable>

class cyclic_barrier
{
	size_t num_of_threads;
	size_t count_of_waiting;
	size_t count_of_passed;
	bool threads_passing;
	
	std::condition_variable all_in;
	std::condition_variable all_passed;
	std::mutex mtx;

public:
	cyclic_barrier(const size_t _num_of_threads) : num_of_threads{ _num_of_threads }, count_of_waiting{ 0 }, count_of_passed{ 0 }, threads_passing{ false } {}

	cyclic_barrier(const cyclic_barrier&) = delete;
	cyclic_barrier& operator=(const cyclic_barrier&) = delete;

	void enter()
	{
		std::unique_lock<std::mutex> lock(mtx);
		all_passed.wait(lock, [this]{ return !threads_passing; });
		count_of_waiting++;
		if (count_of_waiting == num_of_threads)
		{
			threads_passing = true;
			count_of_waiting = 0;
			all_in.notify_all();
		}
		else
		{
			all_in.wait(lock, [this]{ return threads_passing; });
		}
		count_of_passed++;
		if (count_of_passed == num_of_threads)
		{
			count_of_passed = 0;
			threads_passing = false;
			all_passed.notify_all();
		}
	}
};

#endif