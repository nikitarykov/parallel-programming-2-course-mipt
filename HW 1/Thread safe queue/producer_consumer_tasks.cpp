#include <iostream>
#include <thread>
#include "thread_safe_queue.h"

using namespace std;

const int NUM_OF_CONSUMERS = 50;
const int NUM_OF_TASKS = 10000;
const int POISON_PILL = -1;

bool check_for_prime(int val)
{
	if (val <= 1)
		return false;
	for (int i = 2; i * i <= val; ++i)
	{
		if (val % i == 0)
			return false;
	}
	return true;
}

void consumer_do(thread_safe_queue<int>& tasks, mutex& mtx_for_cout)
{
	int val;
	while (true)
	{
		tasks.take(val);
		if (val == POISON_PILL)
			break;
		bool prime = check_for_prime(val);
		lock_guard<mutex> lock(mtx_for_cout);
		if (prime)
			cout << val << " is prime" << endl;
		else
			cout << val << " is not prime" << endl;

	}
}

void producer_do(thread_safe_queue<int>& tasks)
{
	srand(0);
	for (int i = 0; i < NUM_OF_TASKS; ++i)
	{
		int val = rand();
		tasks.push(val);
	}
	for (int i = 0; i < NUM_OF_CONSUMERS; ++i)
		tasks.push(POISON_PILL);
}

int main()
{
	thread_safe_queue<int> tasks;
	vector<thread> consumers;
	mutex mtx_for_cout;
	srand(0);
	for (int i = 0; i < NUM_OF_CONSUMERS; ++i)
		consumers.emplace_back(consumer_do, ref(tasks), ref(mtx_for_cout));
	thread producer(producer_do, ref(tasks));
	for (auto& cur_thread : consumers)
		cur_thread.join();
	producer.join();
	return 0;
}