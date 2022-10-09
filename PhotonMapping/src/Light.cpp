#include "Light.hpp"

#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>

#include "Utils.hpp"
#include "EmbreeWrapper.hpp"

#include "Intersection.hpp"

glm::vec3 Light::_intensityFromPoint(glm::vec3 position, Intersection& intersection, RTCScene scene) const {
  glm::vec3 result{ 0.f };
  auto directionToLight = glm::normalize(position - intersection.position);

  auto shadowRayHit = rtcRayFrom(intersection.position, directionToLight, glm::distance(position, intersection.position));

  struct RTCIntersectContext context;
  rtcInitIntersectContext(&context);

  rtcOccluded1(scene, &context, &shadowRayHit.ray);

    // For some reason, >= 0 means we reached light. tfar = -inf if object is occluded
  if (shadowRayHit.ray.tfar != -std::numeric_limits<float>::infinity()) {
    auto directionModifier = glm::dot(intersection.normal, directionToLight);
    auto diffuse = intersection.material.color * color * std::max(directionModifier, 0.f);

    auto distanceToLight = glm::l2Norm(position, intersection.position);
    auto lightAttenuation = _attenuation(distanceToLight);

    result += lightAttenuation * _intensity * diffuse;
  }

  return result;
}

glm::vec3 PointLight::intensityFrom(Intersection& intersection, RTCScene scene) const {
  return _intensityFromPoint(position, intersection, scene);
}

glm::vec3 AreaLight::intensityFrom(Intersection& intersection, RTCScene scene) const {
  glm::vec3 color{ 0.f };

  auto lightPoints = _lightSourcePoints();

  for (auto lightPoint : lightPoints) {
    color += _intensityFromPoint(lightPoint, intersection, scene);
  }

  return color / (float)lightPoints.size();
}

std::vector<glm::vec3> AreaLight::_lightSourcePoints() const {
  std::vector<glm::vec3> points;
  for (size_t u = 0; u < _usteps; ++u) {
    for (size_t v = 0; v < _vsteps; ++v) {
      points.push_back(_pointOnLight(u, v));
    }
  }

  return points;
}

inline glm::vec3 AreaLight::_pointOnLight(size_t u, size_t v) const {
    // We sample points in random locations allowing softer shadows
  auto uMiddle = (float)u + glm::linearRand(0.f, 1.f);
  auto vMiddle = (float)v + glm::linearRand(0.f, 1.f);
  return _corner + _udirection * uMiddle + _vdirection * vMiddle;
}
