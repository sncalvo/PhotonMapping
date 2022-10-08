#pragma once

#include <memory>

#include <freeimage/FreeImage.h>
#include <glm/glm.hpp>

#include "Color.hpp"


class Image {
public:
  Image(unsigned int width, unsigned int height);

    /// Writes in the desired pixel the color requested
    /// - Parameters:
    ///   - x: horizontal coordinate for the pixel
    ///   - y: vertical coordinate for the pixel
    ///   - color: color to be written
  void writePixel(unsigned int x, unsigned int y, glm::vec3 color);

    /// Saves image to requested path
    /// - Parameter filename: filename/path for the image
  void save(const char* filename);

  ~Image();

  const unsigned int width;
  const unsigned int height;
private:
  FIBITMAP* _bitmap;
  std::unique_ptr<glm::vec3[]> _colorBuffer;
  std::unique_ptr<Color[]> _pixelBuffer;
  glm::vec3 _maxColor;

  void _performGammaCorrection();
};
