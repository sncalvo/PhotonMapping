#pragma once

#include <embree3/rtcore.h>
#include <glm/glm.hpp>

#include "Vector.hpp"
#include "Light.hpp"

inline auto rtcRayFrom(glm::vec3 origin, glm::vec3 direction, float tfar) {
  struct RTCRayHit shadowRayHit;

  shadowRayHit.ray.org_x = origin.x;
  shadowRayHit.ray.org_y = origin.y;
  shadowRayHit.ray.org_z = origin.z;

  shadowRayHit.ray.dir_x = direction.x;
  shadowRayHit.ray.dir_y = direction.y;
  shadowRayHit.ray.dir_z = direction.z;

  shadowRayHit.ray.tnear = 0.0001;
  shadowRayHit.ray.tfar = tfar;

  shadowRayHit.ray.mask = -1;
  shadowRayHit.ray.flags = 0;

  shadowRayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
  shadowRayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

  return shadowRayHit;
}

inline auto rtcRayFrom(glm::vec3 origin, glm::vec3 direction) {
  struct RTCRayHit shadowRayHit;

  shadowRayHit.ray.org_x = origin.x;
  shadowRayHit.ray.org_y = origin.y;
  shadowRayHit.ray.org_z = origin.z;

  shadowRayHit.ray.dir_x = direction.x;
  shadowRayHit.ray.dir_y = direction.y;
  shadowRayHit.ray.dir_z = direction.z;

  shadowRayHit.ray.tnear = 0.0001;
  shadowRayHit.ray.tfar = std::numeric_limits<float>::infinity();

  shadowRayHit.ray.mask = -1;
  shadowRayHit.ray.flags = 0;

  shadowRayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
  shadowRayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

  return shadowRayHit;
}

inline auto rtcRayFrom(Vector vector) {
  struct RTCRayHit shadowRayHit;

  shadowRayHit.ray.org_x = vector.ox;
  shadowRayHit.ray.org_y = vector.oy;
  shadowRayHit.ray.org_z = vector.oz;

  shadowRayHit.ray.dir_x = vector.dx;
  shadowRayHit.ray.dir_y = vector.dy;
  shadowRayHit.ray.dir_z = vector.dz;

  shadowRayHit.ray.tnear = 0.0001;
  shadowRayHit.ray.tfar = std::numeric_limits<float>::infinity();

  shadowRayHit.ray.mask = -1;
  shadowRayHit.ray.flags = 0;

  shadowRayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
  shadowRayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

  return shadowRayHit;
}
