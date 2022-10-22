#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

#include <embree3/rtcore.h>

#include "Model.hpp"
#include "Light.hpp"
#include "Camera.hpp"
#include "BoundingBox.hpp"

/// Class representing scene with list of models
class Scene {
public:
  /// Initializes scene using device by generating an embree scene object
  /// - Parameter device: device for object generation using embree
  Scene(RTCDevice device);
  
  /// Adds model to the scene
  /// - Parameter model: Model to be added
  void addModel(std::shared_ptr<Model> model);
  
  /// Adds bounding box corresponding to a transparent model
  /// Used to target them in the caustics map
  /// - Parameter boundingBox: BoundingBox to be added
  void addTransparentBoundingBox(std::shared_ptr<BoundingBox> boundingBox);
  
  /// Returns all bounding boxes for transparent elements in the scene
  std::vector<std::shared_ptr<BoundingBox>> getTransparentBoundingBoxes();
  
  /// Adds light to the scene
  /// - Parameter light: Light to be added
  void addLight(std::shared_ptr<Light> light);

  /// Returns all lights in the scene
  std::vector<std::shared_ptr<Light>> getLights() const;

  /// Commits scene with models attached
  void commit();

  /// Returns material for the geometry accessed
  Material getMaterial(unsigned int geometryId);

  /// Sets camera that will be used for the scene
  /// - Parameter camera: shared pointer to camera
  void setCamera(std::shared_ptr<Camera> camera);

  /// Gets the camera necessary to render the scene
  std::shared_ptr<Camera> getCamera();

  RTCScene scene;
  glm::vec3 ambient{0.f};

  ~Scene();
private:
  std::vector<std::shared_ptr<Model>> _models;
  std::unordered_map<unsigned int, Material> _materials;
  std::vector<std::shared_ptr<Light>> _lights;
  std::shared_ptr<Camera> _camera;
  std::vector<std::shared_ptr<BoundingBox>> _transparentBoundingBoxes;
};
