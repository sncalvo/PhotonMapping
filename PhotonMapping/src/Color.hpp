#pragma once

#include <freeimage/FreeImage.h>
#include <glm/glm.hpp>

struct Color {
  unsigned char r, g, b, a;

  Color(glm::vec3 color) :
    r((unsigned char)(color.x * 255)),
    g((unsigned char)(color.y * 255)),
    b((unsigned char)(color.z * 255)),
    a(255) {}

  RGBQUAD toRGBQuad() {
    return RGBQUAD {
      r, g, b, a
    };
  };
};
