#pragma once

#include <glm/glm.hpp>

struct Light {
  glm::vec3 position, color;
  float intensity, constantDecay, linearDecay, quadraticDecay;
};
