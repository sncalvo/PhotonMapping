#include "./Image.hpp"

constexpr unsigned int PIXEL_SIZE = 24;

Image::Image(unsigned int width, unsigned int height):
    width(width),
    height(height)
{
    _bitmap = FreeImage_Allocate(width, height, PIXEL_SIZE);

    if (!_bitmap) {
        throw "Failed to create image";
    }
}

void Image::writePixel(unsigned int x, unsigned int y, Color color) {
    if (x >= width || y >= height) {
        throw "Error, trying to write pixel in wrong location";
    }

    RGBQUAD rgbColor = color.toRGBQuad();

    FreeImage_SetPixelColor(_bitmap, x, y, &rgbColor);
}

void Image::save(const char *filename) {
    if (!FreeImage_Save(FIF_PNG, _bitmap, filename, 0)) {
        throw "Error, image not saved correctly";
    };
}

Image::~Image() {
    FreeImage_DeInitialise();
}
