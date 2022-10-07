#include "PhotonMapper.hpp"

#include <glm/gtc/random.hpp>

#include "EmbreeWrapper.hpp"

constexpr unsigned int PHOTON_LIMIT = 10000;

PhotonMapper::PhotonMapper() {
  
}

void PhotonMapper::useScene(std::shared_ptr<Scene> scene) {
  _scene = scene;
}

void PhotonMapper::addLight(std::shared_ptr<Light> light) {
  _lights.push_back(light);
}

void PhotonMapper::makePhotonMap(PhotonMap map) {
  for (auto light : _lights) {
    for (unsigned int i = 0; i < PHOTON_LIMIT; i++) {
      auto direction = glm::sphericalRand(1.f);
      auto position = light->position;

      auto rayIntersection = intersectRay(position, direction, _scene);

      if (!rayIntersection.has_value()) {
        continue;
      }

      auto intersection = rayIntersection.value();
    }
  }
}
