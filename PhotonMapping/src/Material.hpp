#pragma once

#include <string>

struct Texture {
    // TODO: Since this is not opengl, we should have a pointer to the bitmap for the texture
  unsigned int id;
  std::string type;
  std::string path;
};

struct Material {
  Texture texture;
  glm::vec3 color;

  // TODO: Probably add more stuff for ray tracing and photon mapping like material type, different color profiles
  float reflection;
  float transparency;
};
