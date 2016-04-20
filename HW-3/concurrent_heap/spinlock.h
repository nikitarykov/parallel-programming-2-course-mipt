#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <atomic>
#include <thread>

class spinlock
{
	std::atomic<bool> locked;

	spinlock(const spinlock&) = delete;
	spinlock& operator =(const spinlock&) = delete;

public:

	spinlock() {}

	void lock()
	{
		while (locked.exchange(true, std::memory_order_acquire))
		{
			std::this_thread::yield();
		}
	}

	bool try_lock()
	{
		return !locked.exchange(true, std::memory_order_acquire);
	}

	void unlock()
	{
		locked.store(false, std::memory_order_release);
	}
};

#endif
