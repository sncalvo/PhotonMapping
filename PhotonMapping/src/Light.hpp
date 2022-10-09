#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <embree3/rtcore.h>

class Model;

struct Intersection;

class Light {
public:
  virtual glm::vec3 intensityFrom(Intersection& intersection, RTCScene scene) const = 0;
  virtual std::shared_ptr<Model> getModel() const = 0;

  glm::vec3 position, color;

protected:
  Light(glm::vec3 position, glm::vec3 color, float intensity, float constantDecay, float linearDecay, float quadraticDecay) :
    position(position), color(color), _intensity(intensity), _constantDecay(constantDecay), _linearDecay(linearDecay),
    _quadraticDecay(quadraticDecay) {}

  float _intensity, _constantDecay, _linearDecay, _quadraticDecay;

  inline float _attenuation(float distance) const {
    return 1.f / (_constantDecay + _linearDecay * distance + _quadraticDecay * glm::pow(distance, 2.f));
  }

  glm::vec3 _intensityFromPoint(glm::vec3 position, Intersection& intersection, RTCScene scene) const;
};

class PointLight : public Light {
public:
  PointLight(glm::vec3 position, glm::vec3 color, float intensity, float constantDecay, float linearDecay, float quadraticDecay) :
    Light(position, color, intensity, constantDecay, linearDecay, quadraticDecay) {}

  glm::vec3 intensityFrom(Intersection& intersection, RTCScene scene) const;

  std::shared_ptr<Model> getModel() const;
};

class AreaLight : public Light {
public:
  AreaLight(
    glm::vec3 position, glm::vec3 color, float intensity, float constantDecay, float linearDecay,
    float quadraticDecay, glm::vec3 uvec, glm::vec3 vvec, size_t usteps, size_t vsteps, RTCDevice device
  );

  glm::vec3 intensityFrom(Intersection& intersection, RTCScene scene) const;

  std::shared_ptr<Model> getModel() const;

private:
  std::shared_ptr<Model> _model;

  glm::vec3 _corner;

  glm::vec3 _uvec;
  glm::vec3 _vvec;

  glm::vec3 _udirection;
  glm::vec3 _vdirection;

  size_t _usteps;
  size_t _vsteps;

  inline glm::vec3 _pointOnLight(size_t u, size_t v) const;
  std::vector<glm::vec3> _lightSourcePoints() const;
};
