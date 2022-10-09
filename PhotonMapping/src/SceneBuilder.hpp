#include <yaml-cpp/yaml.h>
#include "Scene.hpp"
#include "camera.hpp"
#include "model.hpp"

class SceneBuilder {
public:
  SceneBuilder(RTCDevice device, float aspectRatio);
  std::shared_ptr<Scene> createScene();
private:
  RTCDevice _device;
  float _aspectRatio;
	YAML::Node _file;
	std::vector<std::shared_ptr<Model>> _models;
  std::shared_ptr<Scene> _scene;

  void _loadModels(YAML::Node models);
  void _addSphere(YAML::Node node);
  void _addFileModel(YAML::Node node);
};