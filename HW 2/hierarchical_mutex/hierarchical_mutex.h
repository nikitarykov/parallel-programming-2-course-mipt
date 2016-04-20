#ifndef HIERARCHICAL_MUTEX_H
#define HIERARCHICAL_MUTEX_H

#include <mutex>
#include <thread>

const size_t DEFAULT_MAX_LEVEL = INT_MAX;

class hierarchical_mutex
{
	std::mutex mtx;
	const size_t mtx_level;
	size_t thread_previous_level;
	static _declspec(thread) size_t thread_current_level;

public:
	hierarchical_mutex(const size_t _level = DEFAULT_MAX_LEVEL);

	hierarchical_mutex(const hierarchical_mutex&) = delete;
	hierarchical_mutex& operator=(const hierarchical_mutex&) = delete;

	void lock();
	void unlock();
	bool try_lock();
};

#endif