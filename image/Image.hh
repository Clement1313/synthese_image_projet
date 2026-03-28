#pragma once 

#include "../Colors.hh"
#include <vector>
#include <string>

class Image {
public: 
    int width, height;
    std::vector<Colors> pixels;

    Image(int width, int height): width(width), height(height), pixels(static_cast<size_t>(width * height)) {}

    void setPixel(Colors color, int x, int y);
    void savePPM(std::string path);
};