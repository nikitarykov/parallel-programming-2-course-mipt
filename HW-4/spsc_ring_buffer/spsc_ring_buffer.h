#ifndef SPSC_RING_BUFFER_H
#define SPSC_RING_BUFFER_H

#include <atomic>
#include <vector>

const size_t DEFAULT_BUFFER_SIZE = 10000;
const size_t CACHE_LINE_SIZE = 64;

template<class T>
struct node
{
	T val;
	char pad[CACHE_LINE_SIZE];

	node() {}
	node(T _val) : val(_val) {}
};

template<class T>
class spsc_ring_buffer
{
	using node_t = node<T>;
	const size_t capacity;
	char pad1[CACHE_LINE_SIZE];
	std::atomic<size_t> head;
	char pad2[CACHE_LINE_SIZE];
	std::atomic<size_t> tail;
	char pad3[CACHE_LINE_SIZE];
	std::vector<node_t> data;

	bool full(size_t cur_head, size_t cur_tail)
	{
		return next(cur_tail) == cur_head;
	}

	bool empty(size_t cur_head, size_t cur_tail)
	{
		return cur_tail == cur_head;
	}

	size_t next(size_t index)
	{
		return (index + 1) % capacity;
	}

public:
	spsc_ring_buffer(size_t _capacity = DEFAULT_BUFFER_SIZE) : capacity(_capacity + 1), head(0), tail(0), data(capacity) {}

	bool enqueue(T val)
	{
		size_t cur_head = head.load(std::memory_order_acquire);
		size_t cur_tail = tail.load(std::memory_order_relaxed);
		if (full(cur_head, cur_tail))
		{
			return false;
		}
		data[cur_tail].val = val;
		tail.store(next(cur_tail), std::memory_order_release);
		return true;
	}

	bool dequeue(T& val)
	{
		size_t cur_tail = tail.load(std::memory_order_acquire);
		size_t cur_head = head.load(std::memory_order_relaxed);
		if (empty(cur_head, cur_tail))
		{
			return false;
		}
		val = data[cur_head].val;
		head.store(next(cur_head), std::memory_order_release);
		return true;
	}

	spsc_ring_buffer(const spsc_ring_buffer&) = delete;
	spsc_ring_buffer& operator=(const spsc_ring_buffer&) = delete;
};

#endif