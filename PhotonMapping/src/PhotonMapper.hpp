#pragma once

#include <memory>

#include "Light.hpp"
#include "Scene.hpp"
#include "KDTree.hpp"

enum PhotonMap {
  Caustics, Global, Volumetric
};

struct PhotonHit {
  glm::vec3 position;
  glm::vec3 incidentDirection;
  float radiance;
};

class PhotonMapper {
public:
  PhotonMapper();

  void useScene(std::shared_ptr<Scene> scene);

  void makePhotonMap(PhotonMap map);

  void makeMap(const Camera& camera) const;

private:
//  KDTree _tree;
  std::shared_ptr<Scene> _scene;

  void _shootPhoton(const glm::vec3 origin, const glm::vec3 direction, const Light light, int depth);

  std::vector<PhotonHit> _hits;
};
