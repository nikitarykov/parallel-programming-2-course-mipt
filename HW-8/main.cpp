#include "prefix_sum.h"
#include <chrono>

using namespace std::chrono;

template <class T>
class plus
{
public:
	T operator()(const T& a, const T& b)
	{
		return a + b;
	}
};

int main()
{
	std::vector<long long> a;
	for (long long i = 0; i < 100000; ++i)
		a.push_back(i);
	std::vector<long long> prefix_sums1(a.size());
	std::vector<long long> prefix_sums2 = a;
	steady_clock::time_point t1 = steady_clock::now();
	prefix_sums1[0] = a[0];
	for (size_t i = 1; i < a.size(); ++i)
	{
		prefix_sums1[i] = prefix_sums1[i - 1] + a[i];
	}
	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	std::cout << "consequent prefix sum is done in " << time_span.count() << " seconds" << std::endl;
	plus<long long> obj;
	t1 = steady_clock::now();
	parallel_scan<long long, plus<long long>>(a, obj, prefix_sums2, 8);
	t2 = steady_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	std::cout << "parallel prefix sum is done in " << time_span.count() << " seconds" << std::endl;
	bool ok = true;
	for (size_t i = 0; i < a.size(); ++i)
	{
		if (prefix_sums1[i] != prefix_sums2[i])
		{
			ok = false;
			std::cout << "Error: " << i << "'th sums are not equal" << std::endl;
		}
	}
	if (ok)
	{
		std::cout << "Correct sums" << std::endl;
	}
	return 0;
}