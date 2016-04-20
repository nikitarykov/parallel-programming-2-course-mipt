#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include "thread_safe_queue.h"
#include "threads_guard.h"
#include <future>
#include <functional>

const size_t DEFAULT_NUM_OF_WORKERS = 8;
const size_t DEFAULT_CAPACITY_OF_QUEUE = 10000;

template <typename RET_VAL>
class thread_pool
{
	using task = std::packaged_task<RET_VAL()>;
	using task_func = std::function<RET_VAL()>;
	using async_result = std::future<RET_VAL>;
	
	thread_safe_queue<task> task_queue;
	std::vector<std::thread> workers;
	threads_guard joiner;

	static size_t default_num_of_workers()
	{
		size_t num_of_workers = std::thread::hardware_concurrency();
		if (num_of_workers != 0)
		{
			return num_of_workers;
		}
		return DEFAULT_NUM_OF_WORKERS;
	}

	void worker_do()
	{
		while (1)
		{
			task cur_task;
			task_queue.take(cur_task);
			if (!cur_task.valid())
			{
				task_queue.push(std::move(cur_task));
				break;
			}
			cur_task();
		}
	}

public:
	thread_pool(const size_t num_of_workers = default_num_of_workers(), const size_t capacity_of_queue = DEFAULT_CAPACITY_OF_QUEUE) : joiner{ workers }, task_queue{ capacity_of_queue }
	{
		for (size_t i = 0; i < num_of_workers; ++i)
			workers.emplace_back(&thread_pool::worker_do, this);
	}

	thread_pool(const thread_pool&) = delete;
	thread_pool& operator=(const thread_pool&) = delete;

	async_result submit(task_func task_in)
	{
		task task(task_in);
		async_result ftr = task.get_future();
		task_queue.push(std::move(task));
		return ftr;
	}

	async_result try_submit(task_func task_in)
	{
		task task(task_in);
		async_result ftr = task.get_future();
		task_queue.try_push(std::move(task));
		return ftr;
	}

	void active_wait(async_result res)
	{
		task new_task;
		while (res.wait_for(std::chrono::seconds(0)) != std::future_status::ready && task_queue.try_take(new_task))
		{
			if (!new_task.valid())
			{
				task_queue.push(std::move(new_task));
				break;
			}
			new_task();
		}
	}

	void shutdown()
	{
		task empty_task;
		task_queue.push(std::move(empty_task));
	}

	~thread_pool()
	{
		shutdown();
	}
};

#endif

