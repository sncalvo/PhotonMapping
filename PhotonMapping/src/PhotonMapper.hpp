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

  void makeGlobalPhotonMap(PhotonMap map);

  void makeCausticsPhotonMap(PhotonMap map);

  void makeMap(const Camera& camera) const;

  std::shared_ptr<Kdtree::KdTree> getTree() {
    return _tree;
  }

  std::shared_ptr<Kdtree::KdTree> getCausticsTree() {
    return _caustics_tree;
  }

  void initializeTreeFromFile(std::string photonsTree, std::string causticsTree);

  void saveTreeToFile(std::string photonsTreeFilename, std::string causticsTreeFilename) const;
private:
  std::shared_ptr<Kdtree::KdTree> _tree;
  std::shared_ptr<Kdtree::KdTree> _caustics_tree;
  Kdtree::KdNodeVector _nodes;
  Kdtree::KdNodeVector _caustic_nodes;

  std::shared_ptr<Scene> _scene;

  void _shootPhoton(const glm::vec3 origin, const glm::vec3 direction, const glm::vec3 power, unsigned int depth, bool isCausticMode, bool in);

  void _addHit(PhotonHit photonHit, bool isCausticMode);

  std::vector<PhotonHit> _hits;
  std::vector<PhotonHit> _caustic_hits;
};
