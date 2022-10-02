#include "Scene.hpp"

Scene::Scene(RTCDevice device) {
  scene = rtcNewScene(device);
}

void Scene::addModel(std::shared_ptr<Model> model) {
  _models.push_back(model);
}

void Scene::addLight(Light light) {
  _lights.push_back(light);
}

uint_fast32_t Scene::getNumLights() const {
  return _lights.size();
}

Light Scene::getLight(unsigned int index) const {
  return _lights.at(index);
}

std::vector<Light> Scene::getLights() const {
  return _lights;
}

void Scene::commit() {
  for (auto model : _models) {
    model->commit(scene);
  }
  
  rtcCommitScene(scene);
}

Scene::~Scene() {
  rtcReleaseScene(scene);
}
