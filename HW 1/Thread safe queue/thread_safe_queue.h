#ifndef THREAD_SAFE_QUEUE_H 
#define THREAD_SAFE_QUEUE_H 

#include <mutex>
#include <queue>
#include <condition_variable>

using namespace std;

const int DEFAULT_CAPACITY = 1000;

template <class T>
class thread_safe_queue
{
	const int capacity;
	queue<T> data;
	mutex mtx;
	condition_variable not_full;
	condition_variable not_empty;

	bool full() const
	{
		return data.size() == capacity;
	}

public:
	thread_safe_queue(int _capacity = DEFAULT_CAPACITY) : capacity{ _capacity } {}
	
	thread_safe_queue(const thread_safe_queue&) = delete;
	thread_safe_queue& operator=(const thread_safe_queue&) = delete;

	void take(T& val)
	{
		unique_lock<mutex> lock(mtx);
		not_empty.wait(lock, [this]{ return !data.empty(); });
		val = data.front();
		data.pop();
		not_full.notify_one();
	}

	void push(T val)
	{
		unique_lock<mutex> lock(mtx);
		not_full.wait(lock, [this]{ return !full(); });
		data.push(val);
		not_empty.notify_one();
	}
};

#endif