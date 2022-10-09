#include "PhotonMapper.hpp"

#include <glm/gtc/random.hpp>

#include "EmbreeWrapper.hpp"
#include "Constants.hpp"
#include "Image.hpp"

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
  auto image = Image(IMAGE_WIDTH, IMAGE_HEIGHT);
  auto depthImage = Image(IMAGE_WIDTH, IMAGE_HEIGHT);
  auto coloredImage = Image(IMAGE_WIDTH, IMAGE_HEIGHT);

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

    Kdtree::KdNode node = neighbors->at(0);
    auto photon = node.data;

    auto cameraPointPosition = camera.getProjectionMatrix() * camera.getViewMatrix() * glm::vec4(photon.position, 1.f);

    auto u = image.width - ((cameraPointPosition.x * image.width) / (2.f * cameraPointPosition.w) + image.width / 2.f);
    auto v = (cameraPointPosition.y * image.height) / (2.f * cameraPointPosition.w) + image.height / 2.f;

    if (u >= image.width || u < 0 || v >= image.height || v < 0) {
      continue;
    }

    image.writePixel((unsigned int)u, (unsigned int)v, photon.power);
    depthImage.writePixel((unsigned int)u, (unsigned int)v, glm::vec3 { photon.depth * 40.f });

    delete neighbors;
  }

  for (size_t x = 0; x < coloredImage.width; ++x) {
    for (size_t y = 0; y < coloredImage.height; ++y) {
      auto camera = _scene->getCamera();
      auto direction = camera->pixelRayDirection(x, y, coloredImage.width, coloredImage.height);

      auto result = intersectRay(camera->origin, direction, _scene);
      if (!result.has_value()) {
        coloredImage.writePixel(x, y, _scene->ambient);
        continue;
      }

      auto intersection = result.value();

      Kdtree::KdNodeVector* neighbors = new std::vector<Kdtree::KdNode>();
      std::vector<float> point{ intersection.position.x, intersection.position.y, intersection.position.z };
      _tree->k_nearest_neighbors(point, PHOTONS_PER_SAMPLE, neighbors);

      glm::vec3 average{ 0.f };
      for (auto neighbor : *neighbors) {
        auto weight = 1 + glm::distance(neighbor.data.position, intersection.position);
        average += neighbor.data.power / weight;
      }

      average /= neighbors->size();

      delete neighbors;

      coloredImage.writePixel(x, y, average);
    }
  }

  image.save("photon-hits.jpeg");
  std::cout << "Saved photon-hits.jpeg" << std::endl;
  depthImage.save("photon-hits-depth.jpeg");
  std::cout << "Saved photon-hits-depth.jpeg" << std::endl;
  coloredImage.save("photon-colored.jpeg");
  std::cout << "Saved photon-colored.jpeg" << std::endl;
}

void PhotonMapper::useScene(std::shared_ptr<Scene> scene) {
  _scene = scene;
}

void PhotonMapper::makePhotonMap(PhotonMap map) {
  for (auto light : _scene->getLights()) {
    for (unsigned int i = 0; i < PHOTON_LIMIT; i++) {
      // TODO: We know we won't manage disperse scenes, so let's only generate photons with directions to elements in the scene
      auto direction = randomNormalizedVector();

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
