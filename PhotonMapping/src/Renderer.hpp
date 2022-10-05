#pragma once

#include <optional>

#include <glm/glm.hpp>

#include "Scene.hpp"
#include "Material.hpp"

// Created this to indicate with types when we intend to use the values as color or position
using Color3f = glm::vec3;

struct Intersection {
  // TODO: Add material, normal, position
  Material material;
  glm::vec3 normal;
  glm::vec3 position;
  glm::vec3 direction;
  float distance;
  glm::vec2 uv;
};

/// The Renderer is responsible for creating the image and executing the photon mapping algorithm
class Renderer {
public:
  /// Renders pixel for the coordinate supplied for an image with the width and height provided
  /// - Parameters:
  ///   - x: horizontal coordinate for the requested pixel
  ///   - y: vertical coordinate for the requested pixel
  ///   - width: horizontal size for the image
  ///   - height: vertical size for the image
  Color3f renderPixel(uint_fast32_t x, uint_fast32_t y, uint_fast32_t width, uint_fast32_t height);

  /// Sets the scene used by the renderer
  /// - Parameter scene: shared scene pointer
  void setScene(std::shared_ptr<Scene> scene);

private:
  Color3f _renderPixelSample(uint_fast32_t x, uint_fast32_t y, uint_fast32_t width, uint_fast32_t height);

  Color3f _calculateColor(glm::vec3 origin, glm::vec3 direction, unsigned int depth);

  std::optional<Intersection> _castRay(glm::vec3 origin, glm::vec3 direction);

  Color3f _renderDiffuse(Intersection &intersection);
  Color3f _renderSpecular(Intersection &intersection, unsigned int depth);
  Color3f _renderTransparent(Intersection &intersection, unsigned int depth);

  std::shared_ptr<Scene> _scene;
};
