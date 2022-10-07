#include "Renderer.hpp"

#include <glm/gtx/norm.hpp>

#include "EmbreeWrapper.hpp"

constexpr auto MAX_DEPTH = 5;

float attenuation(float distance, const Light& light) {
  return 1.f / (light.constantDecay + light.linearDecay * distance + light.quadraticDecay * glm::pow(distance, 2.f));
}

void Renderer::setScene(std::shared_ptr<Scene> scene) {
  _scene = scene;
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

  if (intersection.material.diffuse > glm::epsilon<float>()) {
    diffuseColor = _renderDiffuse(intersection);
  }

  if (intersection.material.reflection > glm::epsilon<float>()) {
    specularColor = _renderSpecular(intersection, depth);
  }

  if (intersection.material.transparency > glm::epsilon<float>()) {
    transparentColor = _renderTransparent(intersection, depth);
  }

  return diffuseColor + specularColor + transparentColor;
}

std::optional<Intersection> Renderer::_castRay(glm::vec3 origin, glm::vec3 direction) {
  return intersectRay(origin, direction, _scene);
}

Color3f Renderer::_renderDiffuse(Intersection &intersection) {
  Color3f color{ 0.f };
  for (const auto light : _scene->getLights()) {
    auto directionToLight = glm::normalize(light.position - intersection.position);

    auto shadowRayHit = rtcRayFrom(intersection.position, directionToLight);

    struct RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    rtcOccluded1(_scene->scene, &context, &shadowRayHit.ray);

    // For some reason, >= 0 means we reached light. tfar = -inf if object is occluded
    if (shadowRayHit.ray.tfar != -std::numeric_limits<float>::infinity()) {
      auto directionModifier = glm::dot(intersection.normal, directionToLight);
      // TODO: We have UVs, so we could in theory use a texture here
      // Note that the UVs are for the object coordinates, but could not match coordinates in actual model
      auto diffuse = intersection.material.color * light.color * std::max(directionModifier, 0.f);

      auto distanceToLight = glm::l2Norm(light.position, intersection.position);
      auto lightAttenuation = attenuation(distanceToLight, light);

      color += lightAttenuation * light.intensity * diffuse;
    }
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
    intersection.position + glm::vec3(0.01) * intersection.direction,
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
//      intersection.position + glm::vec3(0.0001) * intersection.direction,
//      refractionDirection, depth - 1
//    ) * intersection.material.transparency;
//  }

  return color;
}
