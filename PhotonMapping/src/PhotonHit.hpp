#pragma once

#include <glm/glm.hpp>

struct PhotonHit {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 incidentDirection;
  glm::vec3 power;
  unsigned int depth;
};
