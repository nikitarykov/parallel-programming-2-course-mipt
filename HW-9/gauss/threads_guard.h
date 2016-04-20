#ifndef THREADS_GUARD_H
#define THREADS_GUARD_H

#include <thread>
#include <vector>

class threads_guard
{
	std::vector<std::thread>& threads;

public:
	threads_guard(std::vector<std::thread>& _threads) : threads{ _threads }  {}

	threads_guard(const threads_guard&) = delete;
	threads_guard& operator=(const threads_guard&) = delete;

	~threads_guard()
	{
		for (auto& cur_thread : threads)
		{
			if (cur_thread.joinable())
				cur_thread.join();
		}
	}
};

#endif