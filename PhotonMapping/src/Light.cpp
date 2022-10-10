#include "Light.hpp"

#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>

#include "Utils.hpp"
#include "EmbreeWrapper.hpp"

#include "Intersection.hpp"
#include "Model.hpp"
#include "Constants.hpp"

std::pair<std::vector<Intersection>, bool> getListOfIntersections(std::shared_ptr<Scene> scene, glm::vec3 origin, glm::vec3 direction, float distance) {
  std::vector<Intersection> result;
  float accumulatedDistance = 0.f;
  size_t iterationCount = -1;

  struct RTCIntersectContext context;
  rtcInitIntersectContext(&context);
  auto shadowRayHit2 = rtcRayFrom(origin, direction, distance);
  rtcOccluded1(scene->scene, &context, &shadowRayHit2.ray);

  do {
    iterationCount += 1;
    struct RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    auto shadowRayHit = rtcRayFrom(origin, direction);

    rtcIntersect1(scene->scene, &context, &shadowRayHit);

    // We did not find any lights. Returning false for light hits
    if (shadowRayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
      accumulatedDistance += shadowRayHit.ray.tfar;

      return std::pair{result, false};
    }

    auto intersection = Intersection{
      shadowRayHit.hit.geomID,
      scene->getMaterial(shadowRayHit.hit.geomID),
      { shadowRayHit.hit.Ng_x, shadowRayHit.hit.Ng_y, shadowRayHit.hit.Ng_z },
      {
        shadowRayHit.ray.org_x + shadowRayHit.ray.dir_x * shadowRayHit.ray.tfar,
        shadowRayHit.ray.org_y + shadowRayHit.ray.dir_y * shadowRayHit.ray.tfar,
        shadowRayHit.ray.org_z + shadowRayHit.ray.dir_z * shadowRayHit.ray.tfar
      },
      {
        shadowRayHit.ray.dir_x,
        shadowRayHit.ray.dir_y,
        shadowRayHit.ray.dir_z
      },
      shadowRayHit.ray.tfar,
      {
        shadowRayHit.hit.u,
        shadowRayHit.hit.v,
      }
    };

    result.push_back(intersection);

    accumulatedDistance += shadowRayHit.ray.tfar;
    origin = intersection.position + direction * EPSILON;

    // Early returning since object is totally occluded
    if (intersection.material.transparency == 0.f && accumulatedDistance < distance - EPSILON) {
      if (shadowRayHit2.ray.tfar != -std::numeric_limits<float>::infinity()) {
        std::cout << "WTF" << std::endl;
      }
      return std::pair{result, false};
    }
  } while (accumulatedDistance < distance - EPSILON);

  return std::pair{result, true};
}

glm::vec3 Light::_intensityFromPoint(glm::vec3 position, Intersection& intersection, std::shared_ptr<Scene> scene) const {
  glm::vec3 result{ 0.f };
  auto directionToLight = glm::normalize(position - intersection.position);
  auto distanceToLight = glm::distance(position, intersection.position);

  auto shadowRayHit = rtcRayFrom(intersection.position, directionToLight, distanceToLight);

  struct RTCIntersectContext context;
  rtcInitIntersectContext(&context);

  // TODO: Change to intersects with all solids until light and check transparency between them
  rtcOccluded1(scene->scene, &context, &shadowRayHit.ray);
  auto lightPathIntersections = getListOfIntersections(scene, intersection.position, directionToLight, distanceToLight);
//
  auto intersections = lightPathIntersections.first;
  auto didReachLight = lightPathIntersections.second;
//
  // Early return since we did not even reach light unoccluded
  if (!didReachLight) {
    return result;
  }
//
  auto s = glm::vec3(1.f);
  // Checking all transclucent objects in path until reaching light
  for (auto intersection : intersections) {
    s *= intersection.material.transparencyColor();

    if (glm::l2Norm(s) <= glm::epsilon<float>()) {
      break;
    }
  }

  // For some reason, >= 0 means we reached light. tfar = -inf if object is occluded
//  if (shadowRayHit.ray.tfar != -std::numeric_limits<float>::infinity()) {
//    auto directionModifier = glm::dot(intersection.normal, directionToLight);
//    auto diffuse = intersection.material.color * color * std::max(directionModifier, 0.f);
//
//    auto lightAttenuation = _attenuation(distanceToLight);
//
//    result += lightAttenuation * _intensity * diffuse;
//  }
  auto directionModifier = glm::dot(intersection.normal, directionToLight);
  auto lightAttenuation = _attenuation(distanceToLight);

  return s * lightAttenuation * _intensity * std::max(directionModifier, 0.f);
//  return result;
}

glm::vec3 PointLight::intensityFrom(Intersection& intersection, std::shared_ptr<Scene> scene) const {
  return _intensityFromPoint(position, intersection, scene);
}

std::shared_ptr<Model> PointLight::getModel() const {
  return nullptr;
}

std::shared_ptr<Model> AreaLight::getModel() const {
  return nullptr;
  // TODO: Remove when shadow checks work
  // return _model;
}

AreaLight::AreaLight(
  glm::vec3 position, glm::vec3 color, float intensity, float constantDecay, float linearDecay,
  float quadraticDecay, glm::vec3 uvec, glm::vec3 vvec, size_t usteps, size_t vsteps, RTCDevice device
) :
  Light(position, color, intensity, constantDecay, linearDecay, quadraticDecay),
  _uvec(uvec), _vvec(vvec), _usteps(usteps), _vsteps(vsteps) {

  _corner = position - vvec * 0.5f - uvec * 0.5f;
  _udirection = uvec / (float)usteps;
  _vdirection = vvec / (float)vsteps;

  _model = std::make_shared<Model>(Material {
      glm::vec3 { 1.f, 1.f, 1.f }, 1.f, 0.f, 0.f, 0.f, true
    }, device, _corner, uvec, vvec);
}

glm::vec3 AreaLight::intensityFrom(Intersection& intersection, std::shared_ptr<Scene> scene) const {
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
