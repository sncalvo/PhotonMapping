#pragma once

#include <freeimage/FreeImage.h>
#include <glm/glm.hpp>

struct Color {
  unsigned char r, g, b, a;

  Color(glm::vec3 color) :
    r((unsigned char)(std::min((int) (color.x * 255), 255))),
    g((unsigned char)(std::min((int) (color.y * 255), 255))),
    b((unsigned char)(std::min((int) (color.z * 255), 255))),
    a(255) {}

  RGBQUAD toRGBQuad() {
    return RGBQUAD {
      r, g, b, a
    };
  };
};
