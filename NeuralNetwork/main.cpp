#include "network.h"
#include "boost/filesystem.hpp"
#include "CImg.h"
#include "steady_timer.h"

const double learning_parameter = 0.01;
const double error_tolerance = 0.1;
const size_t max_cycles = 20;

void load_images(std::vector<std::pair<std::vector<double>, char>>& images)
{
	for (boost::filesystem::recursive_directory_iterator it("./"), end; it != end; ++it)
	{
		if (it->path().extension() == ".bmp")
		{
			cimg_library::CImg<double> cimage = cimg_library::CImg<double>(it->path().string().c_str());
			std::vector<double> image(cimage.height() * cimage.width());
			for (int x = 0; x < cimage.height(); ++x)
			{
				for (int y = 0; y < cimage.width(); ++y)
				{
					double r = cimage(x, y, 0, 0);
					double g = cimage(x, y, 0, 1);
					double b = cimage(x, y, 0, 2);
					image[x * cimage.width() + y] = (0.299 * r + 0.587 * g + 0.114 * b) / 255.0 - 0.5;
				}
			}
			images.push_back(std::make_pair(image, it->path().filename().string()[0]));
		}
	}
}

int main()
{
	std::vector<std::pair<std::vector<double>, char>> images;
	load_images(images);
	std::random_shuffle(images.begin(), images.end());
	size_t training_size = 0.8 * images.size();
	network net;

	steady_timer timer;

	net.training(learning_parameter, error_tolerance, max_cycles, images, training_size);

	const double learning_time = timer.seconds_elapsed();
	std::cout << learning_time << "\n";

	net.testing(images, training_size);
}