#include <iostream>
#include "spsc_ring_buffer.h"
#include <future>
#include <thread>
#include <chrono>
#include <Windows.h>
#include <WinBase.h>

const size_t BUFFER_SIZE = 1024;
const uint64_t DATA_SIZE = 10000000;

using namespace std;
using namespace std::chrono;

uint64_t producer_work(spsc_ring_buffer<uint64_t>& channel)
{
	SetThreadAffinityMask(GetCurrentThread(), 1);
	long long sum = 0;
	for (uint64_t i = 1; i <= DATA_SIZE; ++i)
	{
		while (!channel.enqueue(i))
		{
			this_thread::yield();
		}
		sum += i;
	}
	return sum;
}

uint64_t consumer_work(spsc_ring_buffer<uint64_t>& channel)
{
	SetThreadAffinityMask(GetCurrentThread(), 4);
	uint64_t sum = 0;
	for (uint64_t i = 1; i <= DATA_SIZE; ++i)
	{
		uint64_t val;
		while (!channel.dequeue(val))
		{
			this_thread::yield();
		}
		sum += val;
	}
	return sum;
}

int main()
{
	steady_clock::time_point t1 = steady_clock::now();
	spsc_ring_buffer<uint64_t> channel(BUFFER_SIZE);
	future<uint64_t> produced_sum = async(producer_work, ref(channel));
	future<uint64_t> consumed_sum = async(consumer_work, ref(channel));
	if (produced_sum.get() == consumed_sum.get())
	{
		cout << "RIGHT" << endl;
	}
	else
	{
		cout << "NOT RIGHT" << endl;
	}
	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "runtime = " << time_span.count() << " seconds" << endl;
	return 0;
}