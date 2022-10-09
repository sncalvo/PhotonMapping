#include "Scene.hpp"

Scene::Scene(RTCDevice device) {
  scene = rtcNewScene(device);
}

void Scene::addModel(std::shared_ptr<Model> model) {
  _models.push_back(model);

  auto modelMaterialMap = model->getMaterialsMap();

  _materials.insert(modelMaterialMap.begin(), modelMaterialMap.end());
}

void Scene::addLight(std::shared_ptr<Light> light) {
  _lights.push_back(light);
}

std::vector<std::shared_ptr<Light>> Scene::getLights() const {
  return _lights;
}

void Scene::commit() {
  for (auto model : _models) {
    model->commit(scene);
  }
  
  rtcCommitScene(scene);
}

Material Scene::getMaterial(unsigned int geometryId) {
  return _materials[geometryId];
}

void Scene::setCamera(std::shared_ptr<Camera> camera) {
  _camera = camera;
}

std::shared_ptr<Camera> Scene::getCamera() {
  return _camera;
}

Scene::~Scene() {
  rtcReleaseScene(scene);
}
