#pragma once

#include <memory>

#include "Light.hpp"
#include "Scene.hpp"
//#include "KDTree.hpp"

enum PhotonMap {
  Caustics, Global, Volumetric
};

class PhotonMapper {
public:
  PhotonMapper();

  void addLight(std::shared_ptr<Light> light);
  void useScene(std::shared_ptr<Scene> scene);

  void makePhotonMap(PhotonMap map);

private:
  std::vector<std::shared_ptr<Light>> _lights;
//  KDTree _tree;
  std::vector<glm::vec3> _photonsColor;
  std::shared_ptr<Scene> _scene;
};
