#pragma once

#include <optional>

#include <glm/glm.hpp>

#include "Scene.hpp"
#include "Material.hpp"

using Color3f = glm::vec3;

struct Intersection {
  // TODO: Add material, normal, position
  Material material;
  glm::vec3 normal;
  glm::vec3 position;
  float distance;
};

class Renderer {
public:
  Color3f renderPixel(uint_fast32_t x, uint_fast32_t y, uint_fast32_t width, uint_fast32_t height);

  void setScene(std::shared_ptr<Scene> scene);

private:
  Color3f _renderPixelSample(uint_fast32_t x, uint_fast32_t y, uint_fast32_t width, uint_fast32_t height);

  std::optional<Intersection> _castRay(glm::vec3 origin, glm::vec3 direction);

  Color3f _renderDiffuse(Intersection &intersection);

  std::shared_ptr<Scene> _scene;
};
