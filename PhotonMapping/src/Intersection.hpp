#pragma once

struct Intersection {
    // TODO: Add material, normal, position
  Material material;
  glm::vec3 normal;
  glm::vec3 position;
  glm::vec3 direction;
  float distance;
  glm::vec2 uv;

  Intersection(
               Material material,
               glm::vec3 normal,
               glm::vec3 position,
               glm::vec3 direction,
               float distance,
               glm::vec2 uv
               ) :
  material(material),
  normal(glm::normalize(normal)),
  position(position),
  direction(glm::normalize(direction)),
  distance(distance),
  uv(uv) {}
};
