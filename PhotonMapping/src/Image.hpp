#include "FreeImage/FreeImage.h"

#include "Color.hpp"

class Image {
public:
    Image(unsigned int width, unsigned int height);

    void writePixel(unsigned int x, unsigned int y, Color color);
    void save(const char* filename);

    ~Image();
private:
    FIBITMAP* _bitmap;
    unsigned int _width;
    unsigned int _height;
};
