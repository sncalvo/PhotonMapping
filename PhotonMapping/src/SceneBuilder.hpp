#include <yaml-cpp/yaml.h>
#include "Scene.hpp"
#include "Camera.hpp"
#include "Model.hpp"
#include "BoundingBox.hpp"

class SceneBuilder {
public:
  SceneBuilder(RTCDevice device);
  std::shared_ptr<Scene> createScene();
private:
  RTCDevice _device;
  float _aspectRatio;
	YAML::Node _file;
	std::vector<std::shared_ptr<Model>> _models;
  std::shared_ptr<Scene> _scene;

  void _loadModels(YAML::Node models);
  void _loadLights(YAML::Node lights);
  void _loadConstants(YAML::Node constants);
  void _addSphere(YAML::Node node);
  void _addFileModel(YAML::Node node);
};