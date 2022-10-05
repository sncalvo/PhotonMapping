#include "Renderer.hpp"

#include "Utils.hpp"
#include <glm/gtx/norm.hpp>

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

  Color3f color;

  color = _renderDiffuse(intersection);

  if (intersection.material.reflection > glm::epsilon<float>()) {
    color += _renderSpecular(intersection, depth - 1);
  }

  if (intersection.material.transparency > glm::epsilon<float>()) {
    color += _renderTransparent(intersection, depth - 1);
  }

  return color;
}

std::optional<Intersection> Renderer::_castRay(glm::vec3 origin, glm::vec3 direction) {
  struct RTCIntersectContext context;
  rtcInitIntersectContext(&context);

  auto rayHit = rtcRayFrom(origin, direction);

  rtcIntersect1(_scene->scene, &context, &rayHit);

  if (rayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
    return std::nullopt;
  }

  auto intersection = Intersection{
    _scene->getMaterial(rayHit.hit.geomID),
    { rayHit.hit.Ng_x, rayHit.hit.Ng_y, rayHit.hit.Ng_z },
    {
      rayHit.ray.org_x + rayHit.ray.dir_x * rayHit.ray.tfar,
      rayHit.ray.org_y + rayHit.ray.dir_y * rayHit.ray.tfar,
      rayHit.ray.org_z + rayHit.ray.dir_z * rayHit.ray.tfar
    },
    {
      rayHit.ray.dir_x,
      rayHit.ray.dir_y,
      rayHit.ray.dir_z
    },
    rayHit.ray.tfar,
    {
      rayHit.hit.u,
      rayHit.hit.v,
    }
  };

  return std::make_optional(intersection);
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
      auto intersectionNormal = glm::normalize(intersection.normal);
      auto directionModifier = glm::dot(intersectionNormal, directionToLight);
      // TODO: We have UVs, so we could in theory use a texture here
      // Note that the UVs are for the object coordinates, but could not match coordinates in actual model
      auto diffuse = intersection.material.color * light.color * std::max(directionModifier, 0.f);

      auto distanceToLight = glm::l2Norm(light.position, intersection.position);
      auto lightAttenuation = attenuation(distanceToLight, light);

      color += lightAttenuation * light.intensity * diffuse;
    }
  }

  return color;
}

Color3f Renderer::_renderSpecular(Intersection &intersection, unsigned int depth) {
  if (depth == 0) {
    return Color3f {0.f};
  }
  // Not sure why the -1000.0f. This was taken from the last's year ray tracing
  auto origin = intersection.position;
  auto reflectionDirection = glm::normalize(glm::reflect(glm::normalize(intersection.direction), glm::normalize(intersection.normal)));

  auto color = _calculateColor(origin, reflectionDirection, depth - 1) * intersection.material.reflection;

  return color;
}

Color3f Renderer::_renderTransparent(Intersection &intersection, unsigned int depth) {
  // TODO: Implement transparent
  return Color3f{ 0.4f };
}
