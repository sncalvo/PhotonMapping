#pragma once

#include <freeimage/FreeImage.h>

#include "Color.hpp"

class Image {
public:
    Image(unsigned int width, unsigned int height);
    
    /// Writes in the desired pixel the color requested
    /// - Parameters:
    ///   - x: horizontal coordinate for the pixel
    ///   - y: vertical coordinate for the pixel
    ///   - color: color to be written
    void writePixel(unsigned int x, unsigned int y, Color color);
    
    /// Saves image to requested path
    /// - Parameter filename: filename/path for the image
    void save(const char* filename);

    ~Image();

    const unsigned int width;
    const unsigned int height;
private:
    FIBITMAP* _bitmap;
};
