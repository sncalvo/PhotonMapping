#include "SceneBuilder.hpp"

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

SceneBuilder::SceneBuilder(RTCDevice device) : _device(device) {
}

void SceneBuilder::_addSphere(YAML::Node node) {
  auto material = node["material"].as<Material>();
  auto center = node["center"].as<glm::vec3>();
  auto radius = node["radius"].as<float>();
  auto boundingBox = std::make_shared<BoundingBox>(
    center - glm::vec3(radius),
    center + glm::vec3(radius)
  );
  
  // std::cout << "Bounding box (min): (" << boundingBox.min.x << ", " << boundingBox.min.y << ", " << boundingBox.min.z << ")" << std::endl;
  // std::cout << "Bounding box (max): (" << boundingBox.max.x << ", " << boundingBox.max.y << ", " << boundingBox.max.z << ")" << std::endl;
  
  auto sphere = std::make_shared<Model>(
    RTC_GEOMETRY_TYPE_SPHERE_POINT,
    material,
    _device,
    glm::vec4(center, radius)
  );
  _scene->addModel(sphere);
  _scene->addTransparentBoundingBox(boundingBox);
}

void SceneBuilder::_addFileModel(YAML::Node node) {
    auto material = node["material"].as<Material>();
    auto model = std::make_shared<Model>(node["path"].as<std::string>(), material, _device);
    _scene->addModel(model);
}

void SceneBuilder::_loadModels(YAML::Node models) {
  for (std::size_t i = 0; i < models.size(); i++) {
    if (models[i]["type"].as<std::string>() == "sphere") {
      std::cout << "SPHERE LOADED" << std::endl;
      _addSphere(models[i]);
    } else if (models[i]["type"].as<std::string>() == "fileModel") {
      std::cout << "MODEL LOADED" << std::endl;
      _addFileModel(models[i]);
    }
  }

}

void SceneBuilder::_loadLights(YAML::Node lights) {
  for (std::size_t i = 0; i < lights.size(); i++) {
    auto light = lights[i];
    std::shared_ptr<Light> scene_light;
    if (lights[i]["type"].as<std::string>() == "areaLight") {
      scene_light = std::make_shared<AreaLight>(
        lights[i]["position"].as<glm::vec3>(),
        lights[i]["color"].as<glm::vec3>(),
        lights[i]["intensity"].as<float>(),
        lights[i]["constantDecay"].as<float>(),
        lights[i]["linearDecay"].as<float>(),
        lights[i]["quadraticDecay"].as<float>(),
        lights[i]["uvec"].as<glm::vec3>(),
        lights[i]["vvec"].as<glm::vec3>(),
        lights[i]["usteps"].as<size_t>(),
        lights[i]["vsteps"].as<size_t>(),
        _device
      );
    } else if (lights[i]["type"].as<std::string>() == "pointLight") {

    } else {
      throw("Wrong light type");
    }

    _scene->addLight(scene_light);
  }
}

void SceneBuilder::_loadConstants(YAML::Node constants) {
  if (!constants[EPSILON] ||
      !constants[MAX_PHOTON_SAMPLING_DISTANCE] ||
      !constants[DELTA] ||
      !constants[TOTAL_LIGHT] ||
      !constants[MAX_DEPTH] ||
      !constants[PHOTONS_PER_SAMPLE] ||
      !constants[PHOTON_LIMIT] ||
      !constants[SHOULD_PRINT_CAUSTICS_HIT_PHOTON_MAP] ||
      !constants[SHOULD_PRINT_DEPTH_PHOTON_MAP] ||
      !constants[SHOULD_PRINT_HIT_PHOTON_MAP] ||
      !constants[GAMMA_CORRECTION]) {
      throw("MISSING CONSTANTS");
  }
  std::cout << WIDTH << ": " << constants[WIDTH] << std::endl;
  std::cout << HEIGHT << ": " << constants[HEIGHT] << std::endl;
  std::cout << MAX_PHOTON_SAMPLING_DISTANCE << ": " << constants[MAX_PHOTON_SAMPLING_DISTANCE] << std::endl;
  std::cout << DELTA << ": " << constants[DELTA] << std::endl;
  std::cout << TOTAL_LIGHT << ": " << constants[TOTAL_LIGHT] << std::endl;
  std::cout << MAX_DEPTH << ": " << constants[MAX_DEPTH] << std::endl;
  std::cout << PHOTONS_PER_SAMPLE << ": " << constants[PHOTONS_PER_SAMPLE] << std::endl;
  std::cout << PHOTON_LIMIT << ": " << constants[PHOTON_LIMIT] << std::endl;
  std::cout << SHOULD_PRINT_CAUSTICS_HIT_PHOTON_MAP << ": " << constants[SHOULD_PRINT_CAUSTICS_HIT_PHOTON_MAP] << std::endl;
  std::cout << SHOULD_PRINT_DEPTH_PHOTON_MAP << ": " << constants[SHOULD_PRINT_DEPTH_PHOTON_MAP] << std::endl;
  std::cout << SHOULD_PRINT_HIT_PHOTON_MAP << ": " << constants[SHOULD_PRINT_HIT_PHOTON_MAP] << std::endl;

  INT_CONSTANTS[WIDTH] = constants[WIDTH].as<int>();
  INT_CONSTANTS[HEIGHT] = constants[HEIGHT].as<int>();
  INT_CONSTANTS[MAX_DEPTH] = constants[MAX_DEPTH].as<int>();
  INT_CONSTANTS[PHOTONS_PER_SAMPLE] = constants[PHOTONS_PER_SAMPLE].as<int>();
  INT_CONSTANTS[PHOTON_LIMIT] = constants[PHOTON_LIMIT].as<int>();

  FLOAT_CONSTANTS[EPSILON] = constants[EPSILON].as<float>();
  FLOAT_CONSTANTS[MAX_PHOTON_SAMPLING_DISTANCE] = constants[MAX_PHOTON_SAMPLING_DISTANCE].as<float>();
  FLOAT_CONSTANTS[DELTA] = constants[DELTA].as<float>();
  FLOAT_CONSTANTS[TOTAL_LIGHT] = constants[TOTAL_LIGHT].as<float>();
  FLOAT_CONSTANTS[GAMMA_CORRECTION] = constants[GAMMA_CORRECTION].as<float>();

  BOOL_CONSTANTS[SHOULD_PRINT_CAUSTICS_HIT_PHOTON_MAP] = constants[SHOULD_PRINT_CAUSTICS_HIT_PHOTON_MAP].as<bool>();
  BOOL_CONSTANTS[SHOULD_PRINT_DEPTH_PHOTON_MAP] = constants[SHOULD_PRINT_DEPTH_PHOTON_MAP].as<bool>();
  BOOL_CONSTANTS[SHOULD_PRINT_HIT_PHOTON_MAP] = constants[SHOULD_PRINT_HIT_PHOTON_MAP].as<bool>();
}

std::shared_ptr<Scene> SceneBuilder::createScene() {
  _file = YAML::LoadFile("assets/scene.yaml");

  if (!_file["models"] || !_file["lights"] || !_file["constants"] || !_file["materials"]) {
      throw("MISSING STUFF");
  }
  _scene = std::make_shared<Scene>(_device);

  _loadConstants(_file["constants"]);
  _loadModels(_file["models"]);
  _loadLights(_file["lights"]);

  _scene->commit();

  auto camera = std::make_shared<Camera>(INT_CONSTANTS[WIDTH] / INT_CONSTANTS[HEIGHT], 1.f);
  _scene->setCamera(camera);

  return _scene;
}
