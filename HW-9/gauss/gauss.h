#ifndef GAUSS_H
#define GAUSS_H

#include <vector>

const size_t DEFAULT_NUM_OF_THREADS = 1;

bool gauss(const std::vector<std::vector<double> >& input, std::vector<double>& answer, size_t num_of_threads = DEFAULT_NUM_OF_THREADS);

#endif