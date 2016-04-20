#include <thread>
#include "cyclic_barrier.h"
#include <vector>

using namespace std;


const size_t NUM_OF_THREADS = 20;

void thread_easy_test(cyclic_barrier& barrier, mutex& cout_mutex)
{
	srand(0);
	for (size_t i = 0; i < 10; ++i)
	{
		this_thread::sleep_for(chrono::seconds(rand() % 10 + 1));
		barrier.enter();
		lock_guard<mutex> lock(cout_mutex);
		cout << this_thread::get_id() << endl;
	}
}

void thread_hard_test(cyclic_barrier& barrier)
{
	for (size_t i = 0; i < 100000; ++i)
	{
		barrier.enter();
	}
}

int main()
{
	vector<thread> threads;
	mutex cout_mutex;
	cyclic_barrier barrier(NUM_OF_THREADS);
	for (size_t i = 0; i < NUM_OF_THREADS; ++i)
		threads.emplace_back(thread_easy_test, ref(barrier), ref(cout_mutex));
	for (auto& cur_thread : threads)
		cur_thread.join();
	threads.clear();
	for (size_t i = 0; i < NUM_OF_THREADS; ++i)
		threads.emplace_back(thread_hard_test, ref(barrier));
	for (auto& cur_thread : threads)
		cur_thread.join();
	return 0;
}