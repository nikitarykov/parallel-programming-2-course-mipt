#ifndef PARALLEL_QUICK_SORT_H
#define PARALLEL_QUICK_SORT_H

#include "thread_pool.h"
#include <algorithm>
#include <stack>

const size_t max_depth = 12;

template <class T>
class sort_task
{
	thread_pool<bool>& task_queue;
	std::vector<T>& data;
	size_t left;
	size_t right;
	size_t depth;
	const size_t max_depth;

	size_t partition(std::vector<T>& data, size_t left, size_t right)
	{
		size_t cur_left = left - 1;
		size_t cur_right = right + 1;
		T pivot = data[left];
		while (true)
		{
			while (data[++cur_left] < pivot);
			while (pivot < data[--cur_right]);
			if (cur_left < cur_right)
			{
				std::swap(data[cur_left], data[cur_right]);
			}
			else
			{
				return cur_right;
			}
		}
	}

public:

	sort_task(thread_pool<bool>& _task_queue, std::vector<T>& _data, size_t _left, size_t _right, size_t _depth, size_t _max_depth) : task_queue(_task_queue), data(_data), left(_left), right(_right), depth(_depth), max_depth(_max_depth) {}

	bool operator()()
	{
		if (left >= right)
			return true;
		if (depth > max_depth)
		{
			std::sort(data.begin() + left, data.begin() + right + 1);
		}
		else
		{
			std::stack<std::future<bool>> wait_stack;
			while (depth <= max_depth && left < right)
			{
				size_t middle = partition(data, left, right);

				sort_task sort_left_part(task_queue, data, left, middle, depth + 1, max_depth);
				wait_stack.emplace(task_queue.submit(std::move(sort_left_part)));
				depth++;
				left = middle + 1;
			}
 			std::sort(data.begin() + left, data.begin() + right + 1);
			while (!wait_stack.empty())
			{
				auto it = std::move(wait_stack.top());
				wait_stack.pop();
				task_queue.active_wait(std::move(it));
			}
		}
		return true;
	}

};

template<class T>
void quick_sort(std::vector<T>& data)
{
	thread_pool<bool> pool;
	sort_task<T> sort_whole(pool, data, 0, data.size() - 1, 0, max_depth);
	auto ftr = pool.submit(sort_whole);
	ftr.get();
}

#endif