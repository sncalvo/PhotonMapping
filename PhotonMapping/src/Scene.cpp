#include "Scene.hpp"

Scene::Scene(RTCDevice device) {
    scene = rtcNewScene(device);
}

void Scene::addModel(Model *model) {
    _models.push_back(model);
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
