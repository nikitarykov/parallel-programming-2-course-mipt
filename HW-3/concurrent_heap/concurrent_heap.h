#ifndef CONCURRENT_HEAP_H
#define CONCURRENT_HEAP_H

#include "spinlock.h"
#include <stdexcept>
#include <mutex>
#include <vector>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/lock_types.hpp>

enum mode { inserts, extract_mins };

const bool VALID = true;
const bool EMPTY = false;
const size_t DEFAULT_HEAP_CAPACITY = 10000;
const size_t root = 1;

template<typename T>
struct node
{
	T val;
	bool state;
	spinlock node_lock;

	node() : state(EMPTY) {}
	node(T _val) : val(_val), state(VALID) {}
};

template<typename T>
class concurrent_heap
{
	using node_t = node<T>;
	size_t capacity;
	size_t cur_size;
	spinlock cur_size_lock;
	mode ops_type;
	mutable boost::shared_mutex rw_lock;

	std::vector<node_t> data;

	void sift_up(size_t cur, std::unique_lock<spinlock>& cur_elem_lock)
	{
		size_t parent = cur / 2;
		std::unique_lock<spinlock> upper_elem_lock(data[parent].node_lock);
		while (cur != root && data[parent].val > data[cur].val)
		{
			std::swap(data[parent].val, data[cur].val);
			cur = parent;
			parent = cur / 2;
			cur_elem_lock = std::move(upper_elem_lock);
			upper_elem_lock = std::unique_lock<spinlock>(data[parent].node_lock);
		}
	}

	void sift_down(std::unique_lock<spinlock>& cur_elem_lock)
	{
		size_t cur = root;
		size_t left;
		size_t right;
		while (true)
		{
			left = cur * 2;
			right = cur * 2 + 1;
			std::unique_lock<spinlock> left_elem_lock(data[left].node_lock, std::defer_lock);
			std::unique_lock<spinlock> right_elem_lock(data[right].node_lock, std::defer_lock);
			std::lock(left_elem_lock, right_elem_lock);
			if (data[left].state == VALID && data[right].state == VALID)
			{
				if (data[left].val < data[right].val)
				{
					if (data[left].val < data[cur].val)
					{
						right_elem_lock.unlock();
						std::swap(data[left].val, data[cur].val);
						cur_elem_lock = std::move(left_elem_lock);
						cur = left;
					}
					else
					{
						return;
					}
				}
				else
				{
					if (data[right].val < data[cur].val)
					{
						left_elem_lock.unlock();
						std::swap(data[right].val, data[cur].val);
						cur_elem_lock = std::move(right_elem_lock);
						cur = right;
					}
					else
					{
						return;
					}
				}
			}
			else
			{
				right_elem_lock.unlock();
				if (data[left].state == EMPTY)
				{
					return;
				}
				if (data[left].val < data[cur].val)
				{
					std::swap(data[left].val, data[cur].val);
					cur_elem_lock = std::move(left_elem_lock);
					cur = left;
				}
				else
				{
					return;
				}
			}
		}
	}

public:
	concurrent_heap(const size_t _capacity = DEFAULT_HEAP_CAPACITY, mode _ops_type = inserts) : capacity(_capacity + 1), cur_size(1), ops_type(_ops_type), data(2 * capacity) {}

	concurrent_heap(const concurrent_heap&) = delete;
	concurrent_heap& operator=(const concurrent_heap&) = delete;

	void insert(T val)
	{
		std::unique_lock<boost::shared_mutex> e_lock(rw_lock, std::defer_lock);
		boost::shared_lock<boost::shared_mutex> s_lock(rw_lock, boost::defer_lock);
		if (ops_type == inserts)
		{
			s_lock.lock();
		}
		else
		{
			e_lock.lock();
		}
		std::unique_lock<spinlock> size_lock(cur_size_lock);
		if (cur_size == capacity)
		{
			throw std::out_of_range("Heap is already full!");
		}
		std::unique_lock<spinlock> cur_elem_lock(data[cur_size].node_lock);
		data[cur_size].val = val;
		data[cur_size].state = VALID;
		size_t cur_index = cur_size;
		cur_size++;
		size_lock.unlock();
		sift_up(cur_index, cur_elem_lock);
	}

	bool extract_min(T& val)
	{
		std::unique_lock<boost::shared_mutex> e_lock(rw_lock, std::defer_lock);
		boost::shared_lock<boost::shared_mutex> s_lock(rw_lock, boost::defer_lock);
		if (ops_type == inserts)
		{
			e_lock.lock();
		}
		else
		{
			s_lock.lock();
		}
		std::unique_lock<spinlock> size_lock(cur_size_lock);
		if (cur_size == 1)
		{
			return false;
		}
		cur_size--;
		std::unique_lock<spinlock> root_lock(data[root].node_lock);
		val = data[root].val;
		if (cur_size == 1)
		{
			data[root].state = EMPTY;
			return true;
		}
		std::unique_lock<spinlock> cur_elem_lock(data[cur_size].node_lock);
		data[root].val = data[cur_size].val;
		data[cur_size].state = EMPTY;
		size_lock.unlock();
		cur_elem_lock.unlock();
		sift_down(root_lock);
		return true;
	}

	void change_mode()
	{
		std::unique_lock<boost::shared_mutex> e_lock(rw_lock);
		if (ops_type == inserts)
		{
			ops_type = extract_mins;
		}
		else
		{
			ops_type = inserts;
		}
	}
};

#endif