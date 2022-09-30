#include "./Image.hpp"

constexpr unsigned int PIXEL_SIZE = 24;

Image::Image(unsigned int width, unsigned int height) {
    _width = width;
    _height = height;
    _bitmap = FreeImage_Allocate(width, height, PIXEL_SIZE);

    if (!_bitmap) {
        throw "Failed to create image";
    }
}

void Image::writePixel(unsigned int x, unsigned int y, Color color) {
    if (x >= _width || y >= _height) {
        throw "Error, trying to write pixel in wrong location";
    }

    FreeImage_SetPixelColor(_bitmap, x, y, &color.toRGBQuad());
}

void Image::save(const char *filename) {
    if (!FreeImage_Save(FIF_PNG, _bitmap, filename, 0)) {
        throw "Error, image not saved correctly";
    };
}

Image::~Image() {
    FreeImage_DeInitialise();
}
