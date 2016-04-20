#include "FFT.h"

int main()
{
	size_t n;
	std::cin >> n;
	std::vector<int> A(n);
	for (size_t i = 0; i < n; ++i)
	{
		std::cin >> A[i];
	}
	size_t m;
	std::cin >> m;
	std::vector<int> B(m);
	for (size_t i = 0; i < m; ++i)
	{
		std::cin >> B[i];
	}
	std::vector<int> C;
	multiply_polynomials(A, B, C);
	std::cout << C.size() << std::endl;
	for (auto it : C)
	{
		std::cout << it << " ";
	}
	std::cout << std::endl;
	return 0;
}