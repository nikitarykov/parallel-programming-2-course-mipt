#include <iostream>
#include "hierarchical_mutex.h"
#include <vector>

void thread_do()
{
	hierarchical_mutex mtx1(10000);
	hierarchical_mutex mtx2(20000);
	hierarchical_mutex mtx3(30000);
	hierarchical_mutex mtx4(40000);
	mtx3.lock();
	mtx2.lock();
	mtx2.unlock();
	try
	{
		std::cout << mtx4.try_lock();
	}
	catch (std::exception& fail)
	{
		std::cout << fail.what() << std::endl;
	}
	std::cout << mtx1.try_lock() << std::endl;
	try
	{
		mtx3.unlock();
	}
	catch (std::exception& fail)
	{
		std::cout << fail.what() << std::endl;
	}
	mtx1.unlock();
	try
	{
		mtx4.lock();
	}
	catch (std::exception& fail)
	{
		std::cout << fail.what() << std::endl;
	}
	mtx3.unlock();
}

int main()
{
	std::thread one(thread_do);
	one.join();
	return 0;
}