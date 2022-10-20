#include "./Image.hpp"

#include <glm/gtx/norm.hpp>
#include "Constants.hpp"

constexpr unsigned int PIXEL_SIZE = 24;

Image::Image(unsigned int width, unsigned int height):
width(width),
height(height),
_colorBuffer(std::make_unique<glm::vec3[]>((size_t) height * width)),
_pixelBuffer(std::make_unique<Color[]>((size_t) height * width))
{
  _bitmap = FreeImage_Allocate(width, height, PIXEL_SIZE);

  if (!_bitmap) {
    throw "Failed to create image";
  }
}

void Image::writePixel(unsigned int x, unsigned int y, glm::vec3 color) {
  if (x >= width || y >= height) {
    throw "Error, trying to write pixel in wrong location";
  }

  if (glm::l2Norm(color) > glm::l2Norm(_maxColor)) {
    _maxColor = color;
  }

  _colorBuffer[y * width + x] = color;
}

void Image::save(const char *filename) {
  _performGammaCorrection();

  for (unsigned int x = 0; x < width; ++x) {
    for (unsigned int y = 0; y < height; ++y) {
      auto color = _pixelBuffer[y * width + x];

      RGBQUAD rgbColor = color.toRGBQuad();
      FreeImage_SetPixelColor(_bitmap, x, y, &rgbColor);
    }
  }

  if (!FreeImage_Save(FIF_PNG, _bitmap, filename, 0)) {
    throw "Error, image not saved correctly";
  };
}

Image::~Image() {
    //    FreeImage_DeInitialise();
}

void Image::_performGammaCorrection() {
  auto gamma = FLOAT_CONSTANTS[GAMMA_CORRECTION];
  auto maxNorm = glm::l2Norm(_maxColor);

  for (unsigned int colorIndex = 0; colorIndex < width * height; colorIndex++) {
    glm::vec3 currentColor = _colorBuffer[colorIndex];

    if (currentColor != glm::vec3{0.f}) {
      currentColor /= maxNorm;

      currentColor.x = glm::pow(currentColor.x, 1.f / gamma);
      currentColor.y = glm::pow(currentColor.y, 1.f / gamma);
      currentColor.z = glm::pow(currentColor.z, 1.f / gamma);
    }

    _pixelBuffer[colorIndex] = Color{ currentColor };
  }
}
