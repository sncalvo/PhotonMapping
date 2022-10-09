#pragma once

#include <string>

#include "Utils.hpp"

struct Texture {
    // TODO: Since this is not opengl, we should have a pointer to the bitmap for the texture
  unsigned int id;
  std::string type;
  std::string path;
};

struct Material {
  glm::vec3 color;

  // TODO: Probably add more stuff for ray tracing and photon mapping like material type, different color profiles
  float diffuse;
  float reflection;
  float transparency;

  float refractionIndex = 0.f;

  float diffuseMaxPower(glm::vec3 photonColor) const {
    auto color = diffuseColor();

    auto maxColorIntensity = maxComponent(color.r * photonColor.r, color.g * photonColor.g, color.b * photonColor.b);
    auto maxPhotonColor = maxComponent(photonColor.r, photonColor.g, photonColor.b);

    return maxColorIntensity / maxPhotonColor;
  }

  float specularMaxPower(glm::vec3 photonColor) const {
    auto color = specularColor();

    auto maxColorIntensity = maxComponent(color.r * photonColor.r, color.g * photonColor.g, color.b * photonColor.b);
    auto maxPhotonColor = maxComponent(photonColor.r, photonColor.g, photonColor.b);

    return maxColorIntensity / maxPhotonColor;
  }

  float transparencyMaxPower(glm::vec3 photonColor) const {
    auto color = transparencyColor();

    auto maxColorIntensity = maxComponent(color.r * photonColor.r, color.g * photonColor.g, color.b * photonColor.b);
    auto maxPhotonColor = maxComponent(photonColor.r, photonColor.g, photonColor.b);

    return maxColorIntensity / maxPhotonColor;
  }

  glm::vec3 diffusePower(glm::vec3 photonColor) const {
    return photonColor * diffuseColor() / diffuseMaxPower(diffuseColor());
  }

  glm::vec3 specularPower(glm::vec3 photonColor) const {
    return photonColor * specularColor() / specularMaxPower(diffuseColor());
  }

  glm::vec3 transparencyPower(glm::vec3 photonColor) const {
    return photonColor * transparencyColor() / transparencyMaxPower(diffuseColor());
  }

  glm::vec3 diffuseColor() const {
    return diffuse * color;
  }

  glm::vec3 specularColor() const {
    return reflection * color;
  }

  glm::vec3 transparencyColor() const {
    return transparency * color;
  }
};
