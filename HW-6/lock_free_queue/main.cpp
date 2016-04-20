#include <iostream>
#include "lf_queue.h"
#include <thread>
#include <vector>

using namespace std;

const size_t MAX_THREAD_COUNT = 8;

void work(lf_queue<int>& q)
{
	int pushes = 0;
	int pops = 0;
	for (size_t i = 0; i < 100000; ++i)
	{
		int op = rand() % 2;
		if (op)
		{
			pushes++;
			q.push(rand());
		}
		else
		{
			if (pushes > pops)
			{
				int res = q.pop();
				pops++;
			}
		}
	}
}

int main()
{
	srand(0);
	vector<thread> threads;
	lf_queue<int> q;
	for (size_t i = 0; i < MAX_THREAD_COUNT; ++i)
	{
		threads.emplace_back(work, ref(q));
	}
	for (size_t i = 0; i < MAX_THREAD_COUNT; ++i)
		threads[i].join();
	return 0;
}
