#include "Image.hh"
#include "../Colors.hh"

#include <fstream>
#include <stdexcept>

void Image::savePPM(std::string path) {
	std::ofstream out(path, std::ios::out | std::ios::trunc);

	out << "P3\n";
	out << width << " " << height << "\n";
	out << "255\n";

	auto clamp255 = [](int value) {
		if (value < 0) {
			return 0;
		}
		if (value > 255) {
			return 255;
		}
		return value;
	};

	for (const Colors &pixel : pixels) {
		int r = clamp255(pixel.r);
		int g = clamp255(pixel.g);
		int b = clamp255(pixel.b);
		out << r << " " << g << " " << b << "\n";
	}
}

void Image::setPixel(Colors color, int x, int y) {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return;
	}
	pixels[static_cast<size_t>(y * width + x)] = color;
}

