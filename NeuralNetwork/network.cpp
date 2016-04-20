#include "network.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <random>

const size_t num_of_inputs = 400;
const size_t num_of_outputs = 26;

double activation_function(double input)
{
	return (double)(1 / (1 + exp(-(double)input)));
}

layer::layer(size_t in, size_t out)
{
	num_inputs = in;
	num_outputs = out;
	weights.resize(num_inputs);
	for (size_t i = 0; i < num_inputs; ++i)
	{
		weights[i].resize(num_outputs);
	}
	outputs = new double[num_outputs];
	output_errors = new double[num_outputs];
	expected_values = new double[num_outputs];
}

layer::~layer()
{
	delete[] outputs;
	delete[] output_errors;
	delete[] expected_values;
}

void layer::calc_outputs(thread_pool<bool>& pool)
{
	std::vector<std::future<bool>> results;
	size_t block_size = num_of_outputs % pool.get_num_of_workers() == 0 ? num_of_outputs / pool.get_num_of_workers() : num_of_outputs / pool.get_num_of_workers() + 1;
	for (size_t p = 0; p < pool.get_num_of_workers(); ++p)
	{
		results.emplace_back(pool.submit([&, p, block_size]()
		{
			for (size_t j = p * block_size; j < std::min(num_outputs, (p + 1) * block_size); ++j)
			{
				double acc = 0;
				for (size_t i = 0; i < num_inputs; ++i)
				{
					outputs[j] = weights[i][j] * inputs[i];
					acc += outputs[j];
				}
				outputs[j] = activation_function(acc);
			}
			return true;
		}));
	}
	for (auto& it : results)
	{
		it.get();
	}
}

void layer::calc_errors(thread_pool<bool>& pool)
{
	std::vector<std::future<bool>> results;
	size_t block_size = num_of_inputs % pool.get_num_of_workers() == 0 ? num_of_inputs / pool.get_num_of_workers() : num_of_inputs / pool.get_num_of_workers() + 1;
	for (size_t p = 0; p < pool.get_num_of_workers(); ++p)
	{
		results.emplace_back(pool.submit([&, p, block_size]()
		{
			for (size_t i = p * block_size; i < std::min(num_inputs, (p + 1) * block_size); ++i)
			{
				double acc = 0;
				for (size_t j = 0; j < num_outputs; ++j)
				{
					back_errors[i] = weights[i][j] * output_errors[j];
					acc += back_errors[i];
				}
				back_errors[i] = acc * inputs[i] * (1 - inputs[i]);
			}
			return true;
		}));
	}
	for (auto& it : results)
	{
		it.get();
	}
}

void layer::calc_errors(double& error, thread_pool<bool>& pool)
{
	double total_error = 0;
	for (size_t j = 0; j < num_outputs; ++j)
	{
		output_errors[j] = outputs[j] - expected_values[j];
		total_error += output_errors[j];
	}
	error = total_error;
	calc_errors(pool);
}

void layer::randomize_weights(thread_pool<bool>& pool)
{
	std::random_device rand_dev;
	std::mt19937 mt_engine(rand_dev());
	std::uniform_real_distribution<double> get_double(-0.5, 0.5);
	std::vector<std::future<bool>> results;
	size_t block_size = num_of_inputs % pool.get_num_of_workers() == 0 ? num_of_inputs / pool.get_num_of_workers() : num_of_inputs / pool.get_num_of_workers() + 1;
	for (size_t p = 0; p < pool.get_num_of_workers(); ++p)
	{
		results.emplace_back(pool.submit([&, p, block_size]()
		{
			for (size_t i = p * block_size; i < std::min(num_inputs, (p + 1) * block_size); ++i)
			{
				for (size_t j = 0; j < num_outputs; ++j)
				{
					weights[i][j] = get_double(mt_engine);
				}
			}
			return true;
		}));
	}
	for (auto& it : results)
	{
		it.get();
	}
}

void layer::update_weights(const double coef, thread_pool<bool>& pool)
{
	std::vector<std::future<bool>> results;
	size_t block_size = num_of_inputs % pool.get_num_of_workers() == 0 ? num_of_inputs / pool.get_num_of_workers() : num_of_inputs / pool.get_num_of_workers() + 1;
	for (size_t p = 0; p < pool.get_num_of_workers(); ++p)
	{
		results.emplace_back(pool.submit([&, p, block_size]()
		{
			for (size_t i = p * block_size; i < std::min(num_inputs, (p + 1) * block_size); ++i)
			{
				for (size_t j = 0; j < num_outputs; ++j)
				{
					weights[i][j] -= coef * output_errors[j] * inputs[i];
				}
			}
			return true;
		}));
	}
	for (auto& it : results)
	{
		it.get();
	}
}

network::network() : pool() 
{
	std::cout << "Type in count of layers you want:" << "\n";
	std::cin >> number_of_layers;
	layer_size.resize(number_of_layers);
	layer_size[0] = num_of_inputs;
	layers.emplace_back(new layer(0, layer_size[0]));
	for (size_t i = 1; i < number_of_layers - 1; ++i)
	{
		std::cout << "Type in size of layer " << i << ":\n";
		std::cin >> layer_size[i];
		layers.emplace_back(new layer(layer_size[i - 1], layer_size[i]));
	}
	layer_size[number_of_layers - 1] = num_of_outputs;
	layers.emplace_back(new layer(layer_size[number_of_layers - 2], layer_size[number_of_layers - 1]));
	for (size_t i = 1; i < number_of_layers; ++i)
	{
		layers[i]->inputs = layers[i - 1]->outputs;
		layers[i]->back_errors = layers[i - 1]->output_errors;
		layers[i]->randomize_weights(pool);
	}
}

network::~network()
{
	for (auto it : layers)
	{
		delete it;
	}
}

void network::update_weights(const double coef)
{
	for (size_t i = 1; i < number_of_layers; ++i)
	{
		layers[i]->update_weights(coef, pool);
	}
}

void network::write_outputs(FILE *outfile, size_t& right_answers)
{
	size_t in = 400;
	size_t out = 26;
	char result = 'A';
	char expected_result = 'A';
	double result_value = -1.0;
	double expected_result_value = -1.0;
	for (size_t i = 0; i < out; ++i)
	{
		if (expected_result_value < layers[number_of_layers - 1]->expected_values[i])
		{
			expected_result = 'A' + i;
			expected_result_value = layers[number_of_layers - 1]->expected_values[i];
		}
		if (result_value < layers[number_of_layers - 1]->outputs[i])
		{
			result = 'A' + i;
			result_value = layers[number_of_layers - 1]->outputs[i];
		}
	}
	if (expected_result == result)
	{
		right_answers++;
	}
	fprintf(outfile, "\nresult letter is: %c, expected letter is %c\n", result, expected_result);
}

void network::set_up_pattern(std::pair<std::vector<double>, char>& data)
{
	for (size_t i = 0; i < num_of_inputs; ++i)
		layers[0]->outputs[i] = data.first[i];
	for (size_t i = 0; i < num_of_outputs; i++)
	{
		layers[number_of_layers - 1]->expected_values[i] = 0.0;
	}
	layers[number_of_layers - 1]->expected_values[data.second - 'A'] = 1.0;
}

void network::forward_propagation()
{
	for (size_t i = 1; i < number_of_layers; ++i)
	{
		layers[i]->calc_outputs(pool);
	}
}

void network::backward_propagation(double & total_error)
{
	layers[number_of_layers - 1]->calc_errors(total_error, pool);
	for (size_t i = number_of_layers - 2; i > 0; --i)
	{
		layers[i]->calc_errors(pool);
	}
}

void network::training(const double learning_parameter, const double error_tolerance, const size_t max_cycles, std::vector<std::pair<std::vector<double>, char>>& images, const size_t training_size)
{
	double average_error = 1.0;
	size_t total_cycles = 0;
	while (average_error > error_tolerance && total_cycles < max_cycles)
	{
		double cycle_error = 0;
		double num_patterns = 0;
		for (size_t i = 0; i < training_size; ++i)
		{
			set_up_pattern(images[i]);
			num_patterns++;
			forward_propagation();
			double pattern_error = 0.0;
			backward_propagation(pattern_error);
			cycle_error += pattern_error * pattern_error;
			update_weights(learning_parameter);
		}
		average_error = sqrt((double)cycle_error / (double)num_patterns);
		total_cycles++;
	}
}

void network::testing(std::vector<std::pair<std::vector<double>, char>>& images, const size_t training_size)
{
	FILE* output_file_ptr = output_file_ptr = fopen("output.txt", "w");
	size_t right_answers = 0;
	for (size_t i = training_size; i < images.size(); ++i)
	{
		set_up_pattern(images[i]);
		forward_propagation();
		write_outputs(output_file_ptr, right_answers);
	}
	fprintf(output_file_ptr, "\n%u of %u - %f%%\n", right_answers, images.size() - training_size, 100.0 * (float)right_answers / (float)(images.size() - training_size));
	fclose(output_file_ptr);
}

