#include "hierarchical_mutex.h"
#include <stdexcept>

hierarchical_mutex::hierarchical_mutex(const size_t _level) : mtx_level{ _level }, thread_previous_level{ 0 } {}
_declspec(thread) size_t hierarchical_mutex::thread_current_level(INT_MAX);

void hierarchical_mutex::lock()
{
	if (thread_current_level <= mtx_level)
		throw std::logic_error("Wrong Hierarchy!");
	thread_previous_level = thread_current_level;
	thread_current_level = mtx_level;
	mtx.lock();
}

void hierarchical_mutex::unlock()
{
	if (thread_current_level != mtx_level)
		throw std::logic_error("Wrong Hierarchy!");
	thread_current_level = thread_previous_level;
	mtx.unlock();
}

bool hierarchical_mutex::try_lock()
{
	if (thread_current_level <= mtx_level)
		throw std::logic_error("Wrong Hierarchy!");
	if (!mtx.try_lock())
		return false;
	thread_previous_level = thread_current_level;
	thread_current_level = mtx_level;
	return true;
}