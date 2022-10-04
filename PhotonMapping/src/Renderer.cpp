#include "Renderer.hpp"

#include "Utils.hpp"
#include <glm/gtx/norm.hpp>

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

  auto result = _castRay(camera->origin, direction);

  if (!result.has_value()) {
    return _scene->ambient;
  }

  auto intersection = result.value();

  Color3f color;

  switch (intersection.material.type) {
    case Diffuse:
      color = _renderDiffuse(intersection);
      break;
    case Specular:
      // TODO: Implement specular case
      break;
    case Transparent:
      // TODO: Implement transparent case
      break;
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
    rayHit.ray.tfar
  };

  return std::make_optional(intersection);
}

Color3f Renderer::_renderDiffuse(Intersection &intersection) {
  Color3f color{ 0.f };
  for (const auto light : _scene->getLights()) {
    auto directionToLight = light.position - intersection.position;

    auto shadowRayHit = rtcRayFrom(intersection.position, directionToLight);

    struct RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    rtcOccluded1(_scene->scene, &context, &shadowRayHit.ray);

    // For some reason, >= 0 means we reached light. tfar = -inf if object is occluded
    if (shadowRayHit.ray.tfar >= 0.f) {
      auto diffuse = glm::vec3{0.4f} * light.color * std::max(glm::dot(intersection.normal, directionToLight), 0.f);

      auto distanceToLight = glm::l2Norm(light.position, intersection.position);
      auto lightAttenuation = attenuation(distanceToLight, light);

      color += lightAttenuation * light.intensity * diffuse;
    }
  }

  return color;
}
