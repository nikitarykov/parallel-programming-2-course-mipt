#ifndef TAS_SPINLOCK_H
#define TAS_SPINLOCK_H

#include <atomic>
#include <thread>

class tas_spinlock 
{

	std::atomic<bool> locked;

public:

	tas_spinlock() : locked(false) {}

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

	tas_spinlock(const tas_spinlock&) = delete;
	tas_spinlock& operator=(const tas_spinlock&) = delete;

};

#endif
