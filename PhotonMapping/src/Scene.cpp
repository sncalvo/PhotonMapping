#include "Scene.hpp"

Scene::Scene(RTCDevice device) {
  scene = rtcNewScene(device);
}

void Scene::addModel(std::shared_ptr<Model> model) {
  _models.push_back(model);
}

void Scene::addLight(std::shared_ptr<Light> light) {
  _lights.push_back(light);
}

unsigned int Scene::getNumLights() {
  return _numLights;
}

Light *Scene::getLight(unsigned int index) {
  return _lights.at(index);
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
