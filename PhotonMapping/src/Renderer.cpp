#include "Renderer.hpp"

#include <glm/gtx/norm.hpp>

#include "EmbreeWrapper.hpp"
#include "Constants.hpp"

void Renderer::setScene(std::shared_ptr<Scene> scene) {
  _scene = scene;
}

void Renderer::setTree(std::shared_ptr<Kdtree::KdTree> tree) {
  _tree = tree;
}

glm::vec3 Renderer::renderPixel(
  uint_fast32_t x,
  uint_fast32_t y,
  uint_fast32_t width,
  uint_fast32_t height
) {
  // TODO: Create multiple pixel samples randomly and combine results
  return _renderPixelSample(x, y, width, height);
}

glm::vec3 Renderer::_renderPixelSample(
  uint_fast32_t x,
  uint_fast32_t y,
  uint_fast32_t width,
  uint_fast32_t height
) {
  auto camera = _scene->getCamera();
  auto direction = camera->pixelRayDirection(x, y, width, height);

  return _calculateColor(camera->origin, direction, MAX_DEPTH);
}

Color3f Renderer::_calculateColor(glm::vec3 origin, glm::vec3 direction, unsigned int depth) {
  auto result = _castRay(origin, direction);

  if (!result.has_value()) {
    return _scene->ambient;
  }

  auto intersection = result.value();

  Color3f diffuseColor { 0.f };
  Color3f specularColor { 0.f };
  Color3f transparentColor { 0.f };

  if (intersection.material.diffuse > 0.f) {
    diffuseColor = _renderDiffuse(intersection);
  }

  if (intersection.material.reflection > 0.f) {
    specularColor = _renderSpecular(intersection, depth);
  }

  if (intersection.material.transparency > 0.f) {
    transparentColor = _renderTransparent(intersection, depth);
  }

  Kdtree::KdNodeVector* neighbors = new std::vector<Kdtree::KdNode>();
  std::vector<float> point{ intersection.position.x, intersection.position.y, intersection.position.z };
//  _tree->range_nearest_neighbors(
//    point,
//    MAX_PHOTON_SAMPLING_DISTANCE,
//    neighbors
//  );
  _tree->k_nearest_neighbors(point, PHOTONS_PER_SAMPLE, neighbors);

//  auto predicate = InSameSurfacePredicate(intersection.position, intersection.normal);

  glm::vec3 average{ 0.f };
  for (auto neighbor : *neighbors) {
//    if (!predicate(neighbor)) {
//      continue;
//    }

    auto weight = 1 + glm::distance(neighbor.data.position, intersection.position);
    average += neighbor.data.power / weight;
  }

  average /= neighbors->size();

  delete neighbors;

  return average * 0.2f + (diffuseColor + specularColor + transparentColor) * 0.8f;
}

std::optional<Intersection> Renderer::_castRay(glm::vec3 origin, glm::vec3 direction) {
  return intersectRay(origin, direction, _scene);
}

Color3f Renderer::_renderDiffuse(Intersection &intersection) {
  Color3f color{ 0.f };
  
  for (const auto light : _scene->getLights()) {
    color += light->intensityFrom(intersection, _scene->scene);
  }

  return color * intersection.material.diffuse;
}

Color3f Renderer::_renderSpecular(Intersection &intersection, unsigned int depth) {
  if (depth == 0) {
    return Color3f {0.f};
  }
  // Not sure why the -1000.0f. This was taken from the last's year ray tracing
  auto origin = intersection.position;
  auto reflectionDirection = glm::normalize(glm::reflect(intersection.direction, intersection.normal));

  auto color = _calculateColor(origin, reflectionDirection, depth - 1) * intersection.material.reflection;

  return color;
}

unsigned int invertedNormalCount = 0;
unsigned int nonInvertedNormalCount = 0;

Color3f Renderer::_renderTransparent(Intersection &intersection, unsigned int depth) {
  if (depth == 0) {
    return Color3f { 0.f };
  }

  auto cosTheta = std::min(glm::dot(-intersection.direction, intersection.normal), 1.f);
  auto sinTheta = std::sqrt(1.f - cosTheta * cosTheta);

  auto refractionRatio = intersection.material.refractionIndex;

  Color3f color{ 0.f };

  color += _calculateColor(
    intersection.position + glm::vec3(EPSILON) * intersection.direction,
    intersection.direction, depth - 1
  ) * intersection.material.transparency;

//  if (refractionRatio * sinTheta <= 1.f) {
//    // shorturl.at/cDGZ1
//    glm::vec3 refractionNormal = intersection.normal;
//
//    auto refractionPerpendicular = refractionRatio * (intersection.direction + cosTheta * refractionNormal);
//    auto refractionParallel = -glm::sqrt(
//      std::abs(1.f - glm::dot(refractionPerpendicular, refractionPerpendicular))
//    ) * refractionNormal;
//
//    auto refractionDirection = refractionPerpendicular + refractionParallel;
//    color += _calculateColor(
//      intersection.position + glm::vec3(EPSILON) * intersection.direction,
//      refractionDirection, depth - 1
//    ) * intersection.material.transparency;
//  }

  return color;
}
