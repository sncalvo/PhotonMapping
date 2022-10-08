#include "PhotonMapper.hpp"

#include <glm/gtc/random.hpp>

#include "EmbreeWrapper.hpp"
#include "Constants.hpp"
#include "Image.hpp"

constexpr unsigned int PHOTON_LIMIT = 100000;

PhotonMapper::PhotonMapper() {
  
}

glm::vec3 randomNormalizedVector() {
  while (true) {
    auto x = glm::linearRand(-1.f, 1.f);
    auto y = glm::linearRand(-1.f, 1.f);
    auto z = glm::linearRand(-1.f, 1.f);

    if (x * x + y * y + z * z <= 1) {
      return glm::normalize(glm::vec3(x,y,z));
    }
  }
}

glm::vec3 randomNormalizedVector2() {
  while (true) {
    auto x = glm::linearRand(-1.f, 1.f);
    auto y = glm::linearRand(-1.f, 1.f);
    auto z = glm::linearRand(-1.f, 1.f);

    if (glm::abs(x) < 0.1f || glm::abs(y) < 0.1f || glm::abs(z) < 0.1f) {
      continue;
    }

    if (x * x + y * y + z * z <= 1) {
      return glm::normalize(glm::vec3(x,y,z));
    }
  }
}

void PhotonMapper::makeMap(const Camera& camera) const {
  auto image = Image(1920, 1080);
  auto image2 = Image(1920, 1080);

  for (auto hit : _hits) {
    Kdtree::KdNodeVector* neighbors = new std::vector<Kdtree::KdNode>();

    _tree->k_nearest_neighbors(
      std::vector{
        hit.position.x,
        hit.position.y,
        hit.position.z
      }, 1, neighbors
    );

//    if (hit.power.r > 0.9f && hit.power.g > 0.9f && hit.power.b > 0.9f) {
//      continue;
//    }

//    if (hit.depth != 2) {
//      continue;
//    }

    if (neighbors == nullptr) {
      std::cout << "RIP" << std::endl;
      continue;
    }

    Kdtree::KdNode node = neighbors->at(0);
    auto photon = node.data;

    auto cameraPointPosition = camera.getProjectionMatrix() * camera.getViewMatrix() * glm::vec4(photon.position, 1.f);

    auto u = image.width - ((cameraPointPosition.x * image.width) / (2.f * cameraPointPosition.w) + image.width / 2.f);
    auto v = (cameraPointPosition.y * image.height) / (2.f * cameraPointPosition.w) + image.height / 2.f;

    if (u >= image.width || u < 0 || v >= image.height || v < 0) {
      continue;
    }

    image.writePixel((unsigned int)u, (unsigned int)v, photon.power);
  }

  image.save("photon-test.jpeg");
}

void PhotonMapper::useScene(std::shared_ptr<Scene> scene) {
  _scene = scene;
}

void PhotonMapper::makePhotonMap(PhotonMap map) {
  for (auto light : _scene->getLights()) {
    for (unsigned int i = 0; i < PHOTON_LIMIT; i++) {
      // TODO: We know we won't manage disperse scenes, so let's only generate photons with directions to elements in the scene
      auto direction = randomNormalizedVector();

      if (direction.y > 0.f) {
        direction.y = -direction.y;
      }

      auto position = light.position;

      _shootPhoton(position, direction, light.color, 0);
    }
  }

  _buildKdTree();
}

void PhotonMapper::_buildKdTree() {
  _tree = std::make_shared<Kdtree::KdTree>(&_nodes);
}

void PhotonMapper::_shootPhoton(const glm::vec3 origin, const glm::vec3 direction, const glm::vec3 power, unsigned int depth) {
  auto rayIntersection = intersectRay(origin, direction, _scene);

  if (!rayIntersection.has_value()) {
    return;
  }
  auto intersection = rayIntersection.value();
  auto photonHit = PhotonHit{
    intersection.position,
    intersection.normal,
    intersection.direction,
    power,
    depth
  };

  auto diffuseThreshold = intersection.material.diffuseMaxPower(power);
  auto reflectionThreshold = diffuseThreshold + intersection.material.specularMaxPower(power);
  auto transparencyThreshold = reflectionThreshold + intersection.material.transparencyMaxPower(power);

  auto randomSample = glm::linearRand(0.f, 1.f);

  if (randomSample <= diffuseThreshold) {
    if (depth != 0) {
      _hits.push_back(photonHit);

      _nodes.push_back(Kdtree::KdNode {
        std::vector {
          photonHit.position.x,
          photonHit.position.y,
          photonHit.position.z
        },
        photonHit
      });
    }

    auto reflectionDirection = randomNormalizedVector2();

    if (glm::dot(reflectionDirection, intersection.normal) < 0) {
      reflectionDirection = -reflectionDirection;
    }

    auto reflectionPosition = intersection.position + EPSILON * reflectionDirection;

    _shootPhoton(reflectionPosition, reflectionDirection, intersection.material.diffusePower(power), depth + 1);
  } else if (randomSample <= reflectionThreshold) {
    auto reflectionDirection = glm::reflect(intersection.direction, intersection.normal);

    auto reflectionPosition = intersection.position + EPSILON * reflectionDirection;

    _shootPhoton(reflectionPosition, reflectionDirection, intersection.material.specularPower(power), depth + 1);
  } else if (randomSample <= transparencyThreshold) {
    auto refractionDirection = intersection.direction;

    auto refractionPosition = intersection.position + EPSILON * refractionDirection;

    _shootPhoton(refractionPosition, refractionDirection, intersection.material.transparencyPower(power), depth + 1);
  } else if (depth != 0) {
    _hits.push_back(photonHit);

    _nodes.push_back(Kdtree::KdNode {
      std::vector {
        photonHit.position.x,
        photonHit.position.y,
        photonHit.position.z
      },
      photonHit
    });
  }
}
