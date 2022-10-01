#pragma once

#include "FreeImage/FreeImage.h"

#include "Color.hpp"

class Image {
public:
    Image(unsigned int width, unsigned int height);

    void writePixel(unsigned int x, unsigned int y, Color color);
    void save(const char* filename);

    ~Image();

    const unsigned int width;
    const unsigned int height;
private:
    FIBITMAP* _bitmap;
};
