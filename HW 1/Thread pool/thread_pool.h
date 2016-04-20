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
	using task_func = std::function <RET_VAL()>;
	using task_promise_ptr = std::unique_ptr<std::promise<RET_VAL>>;
	using task = std::pair<task_func, task_promise_ptr>;
	using async_result = std::future<RET_VAL>;
	
	thread_safe_queue<task> task_queue;
	std::vector<std::thread> workers;
	threads_guard joiner;

	static size_t default_num_of_workers()
	{
		size_t num_of_workers = std::thread::hardware_concurrency();
		if (num_of_workers != 0)
			return num_of_workers;
		return DEFAULT_NUM_OF_WORKERS;
	}

	void worker_do()
	{
		while (1)
		{
			task cur_task;
			task_queue.take(cur_task);
			if (!cur_task.second)
			{
				task_queue.push(move(cur_task));
				break;
			}
			try
			{
				RET_VAL res = move(cur_task.first());
				cur_task.second.get()->set_value(res);
			}
			catch (...)
			{
				cur_task.second.get()->set_exception(std::current_exception());
			}
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
		task_promise_ptr ret_value_ptr(new promise<RET_VAL>);
		async_result ftr = ret_value_ptr->get_future();
		task_queue.push(move(make_pair(task_in, move(ret_value_ptr))));
		return ftr;
	}

	async_result try_submit(task_func task_in)
	{
		task_promise_ptr ret_value_ptr(new promise<RET_VAL>);
		async_result ftr = ret_value_ptr->get_future();
		if (!task_queue.try_push(move(make_pair(task_in, move(ret_value_ptr)))))
			async_result kill_ftr = move(ftr);
		return ftr;
	}

	void shutdown()
	{
		task_promise_ptr empty;
		task_func func;
		task_queue.push(move(make_pair(func, move(empty))));
	}

	~thread_pool()
	{
		shutdown();
	}
};

#endif