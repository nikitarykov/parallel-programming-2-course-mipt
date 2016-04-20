#include <iostream>
#include <chrono>
#include <vector>
#include "TAS_spinlock.h"
#include "TATAS_spinlock.h"

using namespace std;
using namespace chrono;

void tas_spinlock_test(tas_spinlock& lock, int& cnt)
{

	for (size_t i = 0; i < 10000000; ++i)
	{
		lock.lock();
		cnt++;
		lock.unlock();
	}
}

void tatas_spinlock_test(tatas_spinlock& lock, int& cnt)
{

	for (size_t i = 0; i < 10000000; ++i)
	{
		lock.lock();
		cnt++;
		lock.unlock();
	}
}

void test_spinlocks(size_t N)
{
	vector<thread> threads;
	tas_spinlock lock1;
	int cnt = 0;
	steady_clock::time_point t1 = steady_clock::now();
	for (size_t i = 0; i < N; ++i)
	{
		threads.emplace_back(tas_spinlock_test, ref(lock1), ref(cnt));
	}
	for (auto& it : threads)
	{
		it.join();
	}
	steady_clock::time_point t2 = steady_clock::now();
	cout << cnt << endl;
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	cout << N << " threads with tas_spinlock time = " << time_span.count() << " seconds" << endl;
	cnt = 0;
	threads.clear();
	tatas_spinlock lock2;
	t1 = steady_clock::now();
	for (size_t i = 0; i < N; ++i)
	{
		threads.emplace_back(tatas_spinlock_test, ref(lock2), ref(cnt));
	}
	for (auto& it : threads)
	{
		it.join();
	}
	t2 = steady_clock::now();
	cout << cnt << endl;
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << N << " threads with tatas_spinlock time = " << time_span.count() << " seconds" << endl;
}

int main()
{
	for (size_t N = 1; N <= 10; ++N)
	{
		test_spinlocks(N);
	}
	return 0;
}