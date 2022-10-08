#include "Camera.hpp"

Camera::Camera(float aspectRatio, float height, float focalLength, glm::vec3 origin) :
  _aspectRatio(aspectRatio),
  _viewportWidth(aspectRatio * height),
  origin(origin),
  _viewportHeight(height),
  _focalLength(focalLength),
  horizontal({aspectRatio * height, 0.f, 0.f}),
  vertical({0.f, height, 0.f}),
  lowerLeftCorner(
    _calculateLowerLeftCorner(
      origin,
      {aspectRatio * height, 0.f, 0.f},
      {0.f, height, 0.f},
      focalLength
    )
  ) {}

glm::vec3 Camera::_calculateLowerLeftCorner(glm::vec3 origin, glm::vec3 horizontal, glm::vec3 vertical, float focalLength) {
  return origin - horizontal * .5f - vertical * .5f + glm::vec3{0.f, 0.f, focalLength};
}
