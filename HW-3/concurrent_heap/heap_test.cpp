#include <iostream>
#include <thread>
#include <cstdint>
#include <atomic>
#include <future>
#include <algorithm>
#include <vector>
#include <random>

#include "steady_timer.h"
#include "barrier.h"

#include "concurrent_heap.h"

//////////////////////////////////////////////////////////////////////////

using uint_t = std::uint64_t;
using heap_t = concurrent_heap<uint_t>;

//////////////////////////////////////////////////////////////////////////

std::vector<uint_t> get_shuffled_uints(const uint_t start, const uint_t step, const uint_t upper_bound) {
	std::vector<uint_t> nums;
	for (uint_t u = start; u <= upper_bound; u += step) {
		nums.push_back(u);
	}
	std::random_shuffle(nums.begin(), nums.end());
	return nums;
}

void insert_nums(heap_t& heap, const uint_t n, const size_t num_threads) {
	std::vector<std::thread> inserters;

	barrier insert_barrier(num_threads);

	for (size_t i = 1; i <= num_threads; ++i) {
		inserters.emplace_back(
			[&, i]() {
			std::vector<uint_t> nums_to_insert = std::move(get_shuffled_uints(i, num_threads, n));

			insert_barrier.enter();

			for (uint_t u : nums_to_insert) {
				heap.insert(u);
			}
		}
		);
	}

	for (auto& t : inserters) {
		t.join();
	}
}

//////////////////////////////////////////////////////////////////////////

const uint_t N = 1000000;
const size_t NUM_INSERT_THREADS = 8;
const size_t NUM_EXTRACT_THREADS = 8;

int main() {
	heap_t heap(N);
	steady_timer timer;

	std::cout << "concurrent inserts -> sequential extract-mins test" << std::endl;

	std::cout << "insert nums..." << std::endl;

	timer.reset();
	insert_nums(heap, N, NUM_INSERT_THREADS);
	std::cout << "completed in " << timer.seconds_elapsed() << " seconds" << std::endl;

	std::cout << "extract nums..." << std::endl;

	// extract all inserted nums in single thread
	// 1, 2, ..., N expected

	timer.reset();

	for (uint_t i = 1; i <= N; ++i) {
		uint_t heap_min;
		if (!heap.extract_min(heap_min)) {
			std::cout << "unexpected empty heap" << std::endl;
			return 1;
		}
		if (heap_min != i) {
			std::cout << "unexpected extract-min result: " << heap_min << ", expected: " << i << std::endl;
			return 1;
		}
	}
	std::cout << "completed in " << timer.seconds_elapsed() << " seconds" << std::endl;

	std::cout << "concurrent inserts -> sequential extract-mins test PASSED" << std::endl;
	std::cout << std::endl;



	std::cout << "concurrent inserts -> concurrent extract-mins test" << std::endl;

	// empty heap expected

	std::cout << "insert nums..." << std::endl;

	timer.reset();
	insert_nums(heap, N, NUM_INSERT_THREADS);
	std::cout << "completed in " << timer.seconds_elapsed() << " seconds" << std::endl;


	std::cout << "extract nums..." << std::endl;

	timer.reset();

	// extract all inserted nums using <NUM_EXTRACT_THREADS> threads

	std::vector<std::future<uint_t>> sums;

	barrier extract_barrier(NUM_EXTRACT_THREADS);

	for (size_t t = 0; t < NUM_EXTRACT_THREADS; ++t) {
		auto extract_and_sum = [&heap, &extract_barrier]() -> uint_t {
			extract_barrier.enter();

			uint_t sum = 0;
			uint_t prev_min = 0;
			uint_t curr_min;
			while (heap.extract_min(curr_min)) {
				if (curr_min <= prev_min) {
					throw std::logic_error("non-monotonic extract-mins!");
				}
				sum += curr_min;
				prev_min = curr_min;
			}
			return sum;
		};
		sums.push_back(std::async(extract_and_sum));
	}

	uint_t extracted_total_sum = 0;
	for (auto& sum_result : sums) {
		extracted_total_sum += sum_result.get();
	}

	std::cout << "completed in " << timer.seconds_elapsed() << " seconds" << std::endl;

	const uint_t inserted_total_sum = (N * (1 + N)) / 2;

	std::cout << "sum of inserted nums:  " << inserted_total_sum << std::endl;
	std::cout << "sum of extracted nums: " << extracted_total_sum << std::endl;

	if (extracted_total_sum == inserted_total_sum) {
		std::cout << "concurrent inserts -> concurrent extract-mins test PASSED" << std::endl;
	}
	else {
		std::cout << "incorrect sum of extracted nums" << std::endl;
		return 1;
	}

	return 0;
}
