#include "thread_pool.h"

using namespace std;

const int NUM_OF_TASKS = 100;

int sum(int a, int b)
{
	return a + b;
}

int exception_test()
{
	throw std::logic_error("Exception! Test failed!");
	return 1;
}

int main()
{
	using namespace placeholders;
	thread_pool<int> tasks;
	vector<future<int>> results;


	for (size_t i = 0; i < NUM_OF_TASKS - 1; ++i)
	{
		function<int()> func = bind(sum, 1, i);
		future<int> ftr = tasks.submit(func);
		results.emplace_back(move(ftr));
	}
	function<int()> func = exception_test;
	future<int> ftr = tasks.submit(func);
	results.emplace_back(move(ftr));
	for (auto& ftr : results)
	{
		try
		{
			cout << "Customer gets " << ftr.get() << endl;
		}
		catch (std::exception &fail)
		{
			cout << fail.what() << endl;
		}
	}
	return 0;
}