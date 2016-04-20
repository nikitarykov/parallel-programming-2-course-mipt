#ifndef THREAD_SAFE_QUEUE_H 
#define THREAD_SAFE_QUEUE_H 

#include <mutex>
#include <queue>
#include <condition_variable>

const size_t DEFAULT_CAPACITY = 1000;

template <class T>
class thread_safe_queue
{
	const size_t capacity;
	std::queue<T> data;
	std::mutex mtx;
	std::condition_variable not_full;
	std::condition_variable not_empty;

	bool full() const
	{
		return data.size() == capacity;
	}

public:
	thread_safe_queue(const size_t _capacity = DEFAULT_CAPACITY) : capacity{ _capacity } {}
	
	thread_safe_queue(const thread_safe_queue&) = delete;
	thread_safe_queue& operator=(const thread_safe_queue&) = delete;

	void take(T& val)
	{
		std::unique_lock<std::mutex> lock(mtx);
		not_empty.wait(lock, [this]{ return !data.empty(); });
		val = move(data.front());
		data.pop();
		not_full.notify_one();
	}

	bool try_take(T& val)
	{
		std::unique_lock<std::mutex> lock(mtx);
		if (data.empty())
		{
			return false;
		}
		not_empty.wait(lock, [this]{ return !data.empty(); });
		val = std::move(data.front());
		data.pop();
		not_full.notify_one();
		return true;
	}

	size_t size()
	{
		std::unique_lock<std::mutex> lock(mtx);
		return data.size();
	}

	void push(T val)
	{
		std::unique_lock<std::mutex> lock(mtx);
		not_full.wait(lock, [this]{ return !full(); });
		data.emplace(move(val));
		not_empty.notify_one();
	}

	bool try_push(T val)
	{
		std::unique_lock<std::mutex> lock(mtx);
		if (full())
		{
			return false;
		}
		data.push_back(move(val));
		not_empty.notify_one();
		return true;
	}
};

#endif