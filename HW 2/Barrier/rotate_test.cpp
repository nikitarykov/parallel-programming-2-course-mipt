#include <thread>
#include <iostream>
#include <vector>

#include "cyclic_barrier.h"

const std::size_t LENGTH = 100;
const std::size_t NUM_ITERS = 998;

void rotate_test() {
	std::vector<int> data;
	for (size_t i = 0; i < LENGTH; ++i) {
		data.push_back(i);
	}

	cyclic_barrier barrier(data.size());

	std::vector<std::thread> threads;
	for (size_t i = 0; i < data.size(); ++i) {
		threads.emplace_back(
			[i, &data, &barrier]() {
			for (size_t k = 0; k < NUM_ITERS; ++k) {
				int next = data[(i + 1) % data.size()];
				barrier.enter();
				data[i] = next;
				barrier.enter();
			}
		}
		);
	}

	for (auto& t : threads) {
		t.join();
	}

	for (size_t i = 0; i < data.size(); ++i) {
		std::cout << i << ": " << data[i] << std::endl;
	}
}

int main() {
	rotate_test();
	return 0;
};