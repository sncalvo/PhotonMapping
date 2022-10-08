#pragma once

#include <memory>

#include "Light.hpp"
#include "Scene.hpp"
#include "KDTree.hpp"
#include "PhotonHit.hpp"

enum PhotonMap {
  Caustics, Global, Volumetric
};

class PhotonMapper {
public:
  PhotonMapper();

  void useScene(std::shared_ptr<Scene> scene);

  void makePhotonMap(PhotonMap map);

  void makeMap(const Camera& camera) const;

  std::shared_ptr<Kdtree::KdTree> getTree() {
    return _tree;
  }

private:
  std::shared_ptr<Kdtree::KdTree> _tree;
  Kdtree::KdNodeVector _nodes;

  std::shared_ptr<Scene> _scene;

  void _buildKdTree();

  void _shootPhoton(const glm::vec3 origin, const glm::vec3 direction, const glm::vec3 power, unsigned int depth);

  std::vector<PhotonHit> _hits;
};
