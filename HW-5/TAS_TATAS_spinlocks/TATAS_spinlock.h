#ifndef TATAS_SPINLOCK_H
#define TATAS_SPINLOCK_H

#include <atomic>
#include <thread>

class tatas_spinlock 
{

	std::atomic<bool> locked;

public:

	tatas_spinlock() : locked(false) {}

	void lock()
	{
		while (true)
		{
			while (locked.load(std::memory_order_acquire))
			{
				std::this_thread::yield();
			}
			if (locked.exchange(true, std::memory_order_acquire))
			{
				std::this_thread::yield();
			}
			else
			{
				break;
			}
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

	tatas_spinlock(const tatas_spinlock&) = delete;
	tatas_spinlock& operator=(const tatas_spinlock&) = delete;

};

#endif