#pragma once

#include <optional>

#include <embree3/rtcore.h>
#include <glm/glm.hpp>

#include "Intersection.hpp"
#include "Scene.hpp"
#include "Utils.hpp"

inline std::optional<Intersection> intersectRay(glm::vec3 origin, glm::vec3 direction, std::shared_ptr<Scene> scene) {
  struct RTCIntersectContext context;
  rtcInitIntersectContext(&context);

  auto rayHit = rtcRayFrom(origin, direction);

  rtcIntersect1(scene->scene, &context, &rayHit);

  if (rayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
    return std::nullopt;
  }

  auto intersection = Intersection{
    scene->getMaterial(rayHit.hit.geomID),
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
