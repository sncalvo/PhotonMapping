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
  
  /// Adds light to the scene
  /// - Parameter light: Light to be added
  void addLight(std::shared_ptr<Light> light);

  /// Returns number of lights in the scene
  unsigned int getNumLights();

  /// Returns the ith light in the scene
  /// - Parameter index: Index of the light to return
  Light *getLight(unsigned int index);

  /// Commits scene with models attached
  void commit();
  
  RTCScene scene;
  
  ~Scene();
private:
  std::vector<std::shared_ptr<Model>> _models;

  unsigned int _numLights;
  std::vector<std::shared_ptr<Light>> _lights;
};
