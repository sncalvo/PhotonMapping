#include "scenebuilder.hpp"

#include <iostream>

namespace YAML {
  template<>
  struct convert<glm::vec3> {
    static Node encode(const glm::vec3& rhs) {
      Node node;
      node.push_back(rhs.x);
      node.push_back(rhs.y);
      node.push_back(rhs.z);
      return node;
    }

    static bool decode(const Node& node, glm::vec3& rhs) {
      if (!node.IsSequence() || node.size() != 3) {
        return false;
      }

      rhs.x = node[0].as<float>();
      rhs.y = node[1].as<float>();
      rhs.z = node[2].as<float>();
      return true;
    }
  };

  template<>
  struct convert<glm::vec4> {
    static Node encode(const glm::vec4& rhs) {
      Node node;
      node.push_back(rhs.x);
      node.push_back(rhs.y);
      node.push_back(rhs.z);
      node.push_back(rhs.w);
      return node;
    }

    static bool decode(const Node& node, glm::vec4& rhs) {
      if (!node.IsSequence() || node.size() != 4) {
        return false;
      }

      rhs.x = node[0].as<float>();
      rhs.y = node[1].as<float>();
      rhs.z = node[2].as<float>();
      rhs.w = node[3].as<float>();
      return true;
    }
  };

  template<>
  struct convert<Material> {
      static Node encode(const Material& rhs) {
          Node node;
          node["color"] = rhs.color;
          node["diffuse"] = rhs.diffuse;
          node["reflection"] = rhs.reflection;
          node["transparency"] = rhs.transparency;
          node["refractionIndex"] = rhs.refractionIndex;
          node["emmisive"] = rhs.emmisive;
          return node;
      }

      static bool decode(const Node& node, Material& rhs) {
          if (!node.IsMap() || node.size() < 4) {
              return false;
          }

          rhs.color = node["color"].as<glm::vec3>();
          rhs.diffuse = node["diffuse"].as<float>();
          rhs.reflection = node["reflection"].as<float>();
          rhs.transparency = node["transparency"].as<float>();
          if (node["refractionIndex"]) {
              rhs.refractionIndex = node["refractionIndex"].as<float>();
          }
          if (node["emmisive"]) {
              rhs.emmisive = node["emmisive"].as<float>();
          }
          return true;
      }
  };
}

SceneBuilder::SceneBuilder(RTCDevice device, float aspectRatio) : _device(device), _aspectRatio(aspectRatio) {
}

//Model SceneBuilder::_addSphere(YAML::Node node) {
//    auto material = node["material"].as<Material>();
//    auto sphere = std::make_shared<Model>(RTC_GEOMETRY_TYPE_SPHERE_POINT, material, _device, node["stuff"].as<glm::vec4>());
//    _scene->addModel(sphere);
//}

void SceneBuilder::_loadModels(YAML::Node models) {
  for (std::size_t i = 0; i < models.size(); i++) {
    if (models[i]["type"].as<std::string>() == "sphere") {
      std::cout << "SPHERE 1" << std::endl;
      //_addSphere(models[i]);
    }
  }

}

/* std::vector<std::shared_ptr<Light>> _loadLights(YAML::Node lights)
{
    std::vector<std::shared_ptr<Light>> result;
    for (std::size_t i = 0; i < lights.size(); i++) {
        result.push_back(std::make_shared<Light>(lights[i].as<Light>()));
    }
    return result;
} */

/* std::shared_ptr<Camera> SceneBuilder::_loadCamera(YAML::Node camera)
{
    camera["aspectRatio"] = this->getAspectRatio();
    return std::make_shared<Camera>(camera.as<Camera>());
} */

std::shared_ptr<Scene> SceneBuilder::createScene() {
  _file = YAML::LoadFile("scene.yaml");
  if (!_file["width"] || !_file["height"] || !_file["models"] || !_file["lights"]) {
      throw("MISSING STUFF");
  }
  _scene = std::make_shared<Scene>(_device);

  _loadModels(_file["models"]);
  auto floor = std::make_shared<Model>("./assets/plane.obj", Material { glm::vec3 { 1.f, 1.f, 1.f }, 0.9f, 0.f, 0.f,  }, _device);
  auto sphere2 = std::make_shared<Model>(RTC_GEOMETRY_TYPE_SPHERE_POINT, Material { glm::vec3 { 0.8f, 0.8f, 0.8f }, 0.0f, 0.9f, 0.f, }, _device, glm::vec4 { 2.0f, -2.0f, 7.0f, 1.0f });
  auto backWall = std::make_shared<Model>("./assets/backwall.obj", Material { glm::vec3 { 1.f, 1.f, 1.f }, 0.9f, 0.f, 0.f,  }, _device);
  auto leftWall = std::make_shared<Model>("./assets/leftwall.obj", Material { glm::vec3 { 1.f, 0.f, 0.f }, 0.9f, 0.f, 0.f,  }, _device);
  auto rightWall = std::make_shared<Model>("./assets/rightwall.obj", Material { glm::vec3 { 0.f, 0.f, 1.f }, 0.9f, 0.f, 0.f,  }, _device);
  auto ceiling = std::make_shared<Model>("./assets/ceiling.obj", Material { glm::vec3 { 1.f, 1.f, 1.f }, 0.9f, 0.f, 0.f,  }, _device);

  std::shared_ptr<Light> light = std::make_shared<AreaLight>(
    glm::vec3 {0.0f, 3.9f, 7.f},
    glm::vec3 {1.f, 1.f, 1.f},
    1.f,
    0.2f,
    0.09f,
    0.042f,
    glm::vec3{0.5, 0.f, 0.f},
    glm::vec3{0.f, 0.f, 0.5f},
    3,
    2,
    _device
  );

  std::shared_ptr<Light> light2 = std::make_shared<AreaLight>(
    glm::vec3 {-3.f, 0.f, 6.f},
    glm::vec3 {1.f, 1.f, 1.f},
    1.f,
    0.2f,
    0.09f,
    0.042f,
    glm::vec3{0.0f, 0.5f, 0.f},
    glm::vec3{0.f, 0.f, 0.5},
    3,
    2,
    _device
  );

  _scene->addModel(floor);
/*   scene->addModel(ball1); */
/*   scene->addModel(ball2); */
  _scene->addModel(sphere2);
  _scene->addModel(backWall);
  _scene->addModel(leftWall);
  _scene->addModel(rightWall);
  _scene->addModel(ceiling);
  _scene->addLight(light);
  _scene->addLight(light2);
  _scene->commit();

  auto camera = std::make_shared<Camera>(_aspectRatio, 1.f);
  _scene->setCamera(camera);

  /*_lights = _loadLights(_file["lights"]);
    _camera = _loadCamera(_file["camera"]); */
  return _scene;
}

/* int SceneBuilder::getWidth() const
{
    return _file["width"].as<int>();
}

int SceneBuilder::getHeight() const
{
    return _file["height"].as<int>();
}

int SceneBuilder::getDepth() const
{
    return _file["depth"].as<int>();
}

int SceneBuilder::getSamples() const
{
    return _file["samples"].as<int>();
}

std::vector<std::shared_ptr<Model>> SceneBuilder::getModels() const
{
    return _models;
}

std::vector<std::shared_ptr<Light>> SceneBuilder::getLights() const
{
    return _lights;
}

float SceneBuilder::getAspectRatio() const
{
    return (float) this->getWidth() / (float) this->getHeight();
}

std::shared_ptr<Camera> SceneBuilder::getCamera() const
{
    return _camera;
} */