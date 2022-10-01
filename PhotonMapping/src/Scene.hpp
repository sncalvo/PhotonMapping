#pragma once

#include <vector>
#include <memory>

#include <embree3/rtcore.h>

#include "Model.hpp"

/// Class representing scene with list of models
class Scene {
public:
    /// Initializes scene using device by generating an embree scene object
    /// - Parameter device: device for object generation using embree
    Scene(RTCDevice device);
    
    /// Adds model to the scene
    /// - Parameter model: Model to be added
    void addModel(std::shared_ptr<Model> model);
    
    /// Commits scene with models attached
    void commit();
    
    RTCScene scene;
    
    ~Scene();
private:
    std::vector<std::shared_ptr<Model>> _models;
};
