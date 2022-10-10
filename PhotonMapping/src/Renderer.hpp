#pragma once

#include <optional>

#include <glm/glm.hpp>

#include "Scene.hpp"
#include "Material.hpp"
#include "KDTree.hpp"
#include "Intersection.hpp"

  // Created this to indicate with types when we intend to use the values as color or position
using Color3f = glm::vec3;

struct InSameSurfacePredicate : Kdtree::KdNodePredicate {
  glm::vec3 position;
  glm::vec3 normal;

  InSameSurfacePredicate(glm::vec3 position, glm::vec3 normal) :
    position(position),
    normal(normal) {}

  bool operator()(const Kdtree::KdNode& node) const {
    if (node.data.normal != normal) {
      return false;
    }

    auto directionToPhoton = node.data.position - position;

    return glm::abs(glm::dot(directionToPhoton, normal)) < FLOAT_CONSTANTS[EPSILON];
  }
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
  Color3f renderPixel(uint_fast32_t x, uint_fast32_t y, uint_fast32_t width, uint_fast32_t height, Color3f* pmColor);

    /// Sets the scene used by the renderer
    /// - Parameter scene: shared scene pointer
  void setScene(std::shared_ptr<Scene> scene);

  void setTree(std::shared_ptr<Kdtree::KdTree> tree);

  void setCausticsTree(std::shared_ptr<Kdtree::KdTree> tree);

private:
  Color3f _renderPixelSample(uint_fast32_t x, uint_fast32_t y, uint_fast32_t width, uint_fast32_t height, Color3f* pmColor);

  Color3f _calculateColor(glm::vec3 origin, glm::vec3 direction, unsigned int depth, Color3f* pmColor, bool in);

  std::optional<Intersection> _castRay(glm::vec3 origin, glm::vec3 direction);

  Color3f _renderDiffuse(Intersection &intersection);
  Color3f _renderSpecular(Intersection &intersection, unsigned int depth, Color3f* pmColor, bool in);
  Color3f _renderTransparent(Intersection &intersection, unsigned int depth, Color3f* pmColor, bool in);
  
  Color3f _computeRadianceWithPhotonMap(Intersection &intersection);

  std::shared_ptr<Scene> _scene;
  std::shared_ptr<Kdtree::KdTree> _tree;
  std::shared_ptr<Kdtree::KdTree> _caustics_tree;
};
