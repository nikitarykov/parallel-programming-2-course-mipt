#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include "thread_pool.h"
#include "steady_timer.h"

class network;

class layer
{
protected:
	size_t num_inputs;
	size_t num_outputs;
	std::vector<std::vector<double> > weights;
	double* inputs;
	double* outputs;
	double* back_errors;
	double* output_errors;
	double* expected_values;
	friend network;

public:
	layer(size_t in, size_t out);
	~layer();

	void calc_outputs(thread_pool<bool>& pool);
	void calc_errors(double& error, thread_pool<bool>& pool);
	void calc_errors(thread_pool<bool>& pool);
	void randomize_weights(thread_pool<bool>& pool);
	void update_weights(const double coef, thread_pool<bool>& pool);
};

class network
{
	std::vector<layer*> layers;
	size_t number_of_layers;
	std::vector<int> layer_size;
	thread_pool<bool> pool;

	void update_weights(const double coef);
	void write_outputs(FILE *outfile, size_t& right_answers);
	void set_up_pattern(std::pair<std::vector<double>, char>& data);
	void forward_propagation();
	void backward_propagation(double & total_error);

public:
	network();
	~network();
	
	void training(const double learning_parameter, const double error_tolerance, const size_t max_cycles, std::vector<std::pair<std::vector<double>, char>>& images, const size_t training_size);
	void testing(std::vector<std::pair<std::vector<double>, char>>& images, const size_t training_size);
};

#endif
