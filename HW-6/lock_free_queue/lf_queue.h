#ifndef LF_QUEUE_H
#define LF_QUEUE_H

#include <atomic>

template<class T>
struct node
{
	T data;
	std::atomic<node*> next;

	node(const T& _data) : data(_data), next(nullptr) {}
};

template<class T>
class lf_queue
{
	using node_t = node < T > ;
	std::atomic<node_t*> head;
	std::atomic<node_t*> tail;
	std::atomic<size_t> threads_in_pop;
	std::atomic<node_t*> to_delete;

public:

	lf_queue() : head(new node_t(0)), threads_in_pop(0), to_delete(nullptr)
	{
		tail.store(head.load(std::memory_order_relaxed));
	}
	
	lf_queue(const lf_queue&) = delete;
	lf_queue& operator=(const lf_queue&) = delete;

	void push(const T& _data)
	{
		node_t* new_node = new node_t(_data);
		node_t*	cur_tail;
		node_t*	cur_tail_next;
		while (true)
		{
			cur_tail = tail.load(std::memory_order_acquire);
			cur_tail_next = cur_tail->next.load(std::memory_order_acquire);
			if (cur_tail_next != nullptr)
			{
				tail.compare_exchange_weak(cur_tail, cur_tail_next, std::memory_order_release, std::memory_order_relaxed);
				continue;
			}
			node_t* null_node = nullptr;
			if (cur_tail->next.compare_exchange_weak(null_node, new_node, std::memory_order_release, std::memory_order_relaxed))
			{
				break;
			}
		}
		tail.compare_exchange_strong(cur_tail, new_node, std::memory_order_release, std::memory_order_relaxed);
	}

	void push_to_delete(node_t* next_to_delete)
	{
		node_t* next_to_delete_next = to_delete.load(std::memory_order_acquire);
		next_to_delete->next.store(next_to_delete_next, std::memory_order_release);
		while (!to_delete.compare_exchange_weak(next_to_delete_next, next_to_delete))
		{
			next_to_delete->next.store(next_to_delete_next, std::memory_order_release);
		}
	}

	void try_delete_nodes(node_t* old_head)
	{
		if (threads_in_pop.load(std::memory_order_acquire) == 1)
		{
			node_t* cur_node = to_delete.exchange(nullptr, std::memory_order_acquire);
			if (threads_in_pop.load(std::memory_order_acquire) == 1)
			{
				while (cur_node != nullptr)
				{
					node_t* next_node = cur_node->next.load(std::memory_order_acquire);
					delete cur_node;
					cur_node = next_node;
				}
			}
			else
			{
				if (cur_node != nullptr)
				{
					node_t* new_to_delete = cur_node;
					node_t* next_node = cur_node->next.load(std::memory_order_acquire);
					while (next_node != nullptr)
					{
						cur_node = next_node;
						next_node = cur_node->next.load(std::memory_order_acquire);
					}
					node_t* cur_node_next = to_delete.load(std::memory_order_acquire);
					cur_node->next.store(cur_node_next, std::memory_order_release);
					while (!to_delete.compare_exchange_weak(cur_node_next, new_to_delete, std::memory_order_acquire, std::memory_order_relaxed))
					{
						cur_node->next.store(cur_node_next, std::memory_order_release);
					}
				}
			}
			delete old_head;
		}
		else
		{
			push_to_delete(old_head);
		}
	}

	T pop()
	{
		threads_in_pop.fetch_add(1, std::memory_order_acquire);
		node_t* old_head;
		node_t* old_head_next;
		while (true)
		{
			old_head = head.load(std::memory_order_acquire);
			old_head_next = old_head->next.load(std::memory_order_acquire);
			if (old_head_next == nullptr)
			{
				return old_head->data;
			}
			node_t* cur_tail = tail.load(std::memory_order_acquire);
			if (old_head == cur_tail)
			{
				tail.compare_exchange_strong(cur_tail, cur_tail->next.load(std::memory_order_acquire), std::memory_order_acquire, std::memory_order_relaxed);
				continue;
			}
			if (head.compare_exchange_weak(old_head, old_head_next, std::memory_order_acquire, std::memory_order_relaxed))
			{
				break;
			}
			try_delete_nodes(old_head);
			threads_in_pop.fetch_add(-1, std::memory_order_acquire);
		}
		return old_head_next->data;
	}
};

#endif