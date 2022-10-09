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
  auto causticsImage = Image(IMAGE_WIDTH, IMAGE_HEIGHT);
  auto depthImage = Image(IMAGE_WIDTH, IMAGE_HEIGHT);

  if (SHOULD_PRINT_HIT_PHOTON_MAP || SHOULD_PRINT_DEPTH_PHOTON_MAP) {
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

      if (SHOULD_PRINT_HIT_PHOTON_MAP)
        image.writePixel((unsigned int)u, (unsigned int)v, photon.power);

      if (SHOULD_PRINT_DEPTH_PHOTON_MAP)
        depthImage.writePixel((unsigned int)u, (unsigned int)v, glm::vec3 { photon.depth * 40.f });

      delete neighbors;
    }

    if (SHOULD_PRINT_HIT_PHOTON_MAP) {
      image.save("photon-hits.jpeg");
      std::cout << "Saved photon-hits.jpeg" << std::endl;
    }

    if (SHOULD_PRINT_DEPTH_PHOTON_MAP) {
      depthImage.save("photon-hits-depth.jpeg");
      std::cout << "Saved photon-hits-depth.jpeg" << std::endl;
    }
  }

  if (SHOULD_PRINT_CAUSTICS_HIT_PHOTON_MAP) {
    for (auto hit : _caustic_hits) {
      Kdtree::KdNodeVector* caustic_neighbors = new std::vector<Kdtree::KdNode>();

      _caustics_tree->k_nearest_neighbors(
        std::vector{
          hit.position.x,
          hit.position.y,
          hit.position.z
        }, 1, caustic_neighbors
      );

  //    if (hit.power.r > 0.9f && hit.power.g > 0.9f && hit.power.b > 0.9f) {
  //      continue;
  //    }

      Kdtree::KdNode node = caustic_neighbors->at(0);
      auto photon = node.data;

      auto cameraPointPosition = camera.getProjectionMatrix() * camera.getViewMatrix() * glm::vec4(photon.position, 1.f);

      auto u = image.width - ((cameraPointPosition.x * image.width) / (2.f * cameraPointPosition.w) + image.width / 2.f);
      auto v = (cameraPointPosition.y * image.height) / (2.f * cameraPointPosition.w) + image.height / 2.f;

      if (u >= image.width || u < 0 || v >= image.height || v < 0) {
        continue;
      }

      causticsImage.writePixel((unsigned int)u, (unsigned int)v, photon.power);

      delete caustic_neighbors;
    }

    causticsImage.save("caustics-photon-hits.jpeg");
    std::cout << "Saved caustics-photon-hits.jpeg" << std::endl;
  }
}

void PhotonMapper::useScene(std::shared_ptr<Scene> scene) {
  _scene = scene;
}

void PhotonMapper::makeGlobalPhotonMap(PhotonMap map) {
  for (auto light : _scene->getLights()) {
    for (unsigned int i = 0; i < PHOTON_LIMIT; i++) {
      // TODO: We know we won't manage disperse scenes, so let's only generate photons with directions to elements in the scene
      auto direction = randomNormalizedVector();

      auto position = light->position;

      _shootPhoton(position, direction, light->color, 0, false, false);
    }
  }

  _tree = std::make_shared<Kdtree::KdTree>(&_nodes);
}

void PhotonMapper::makeCausticsPhotonMap(PhotonMap map) {
  for (auto light : _scene->getLights()) {
    // We use a lot more photons for caustics, since most get discarded
    for (unsigned int i = 0; i < PHOTON_LIMIT * 50; i++) {
      // TODO: We know we won't manage disperse scenes, so let's only generate photons with directions to elements in the scene
      auto direction = randomNormalizedVector();

      auto position = light->position;

      _shootPhoton(position, direction, light->color, 0, true, false);
    }
  }

  _caustics_tree = std::make_shared<Kdtree::KdTree>(&_caustic_nodes);
}

void PhotonMapper::_shootPhoton(const glm::vec3 origin, const glm::vec3 direction,
                                const glm::vec3 power, unsigned int depth, bool isCausticMode, bool in) {
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
      _addHit(photonHit, isCausticMode);
    }

    if (!isCausticMode) {
      auto reflectionDirection = randomNormalizedVector2();

      if (glm::dot(reflectionDirection, intersection.normal) < 0) {
        reflectionDirection = -reflectionDirection;
      }

      auto reflectionPosition = intersection.position + EPSILON * reflectionDirection;

      _shootPhoton(reflectionPosition, reflectionDirection, intersection.material.diffusePower(power), depth + 1, isCausticMode, in);
    }
  } else if (randomSample <= reflectionThreshold) {
    if (!isCausticMode) {
      auto reflectionDirection = glm::reflect(intersection.direction, intersection.normal);
      auto reflectionPosition = intersection.position + EPSILON * reflectionDirection;

      _shootPhoton(reflectionPosition, reflectionDirection, intersection.material.specularPower(power), depth + 1, isCausticMode, in);
    }
  } else if (randomSample <= transparencyThreshold || in) {
    auto cosTita = glm::dot(-intersection.normal, intersection.direction);
    auto normal = intersection.normal;
    float nuIt;
    if (cosTita < 0) {
      cosTita = glm::dot(intersection.normal, intersection.direction);
      normal = -intersection.normal;
    }

    if (in) {
      nuIt = intersection.material.refractionIndex / 1.0;
    } else {
      nuIt = 1.0 / intersection.material.refractionIndex;
    }

    float Ci = cosTita;
		float SiSqrd = 1 - pow(Ci, 2);
		float discriminant = 1 - pow(nuIt, 2) * SiSqrd;
		if (discriminant < 0) {
			return; //Total Internal Reflection case
		}
		else {
			glm::vec3 refractionDirection = nuIt * intersection.direction + (Ci * nuIt - sqrtf(discriminant)) * normal;
      auto refractionPosition = intersection.position + EPSILON * refractionDirection;
      _shootPhoton(intersection.position, refractionDirection, intersection.material.transparencyPower(power), depth + 1, isCausticMode, !in);
		}
  } else if (depth != 0) {
    _addHit(photonHit, isCausticMode);
  }
}

void PhotonMapper::_addHit(PhotonHit photonHit, bool isCausticMode) {
  auto node = Kdtree::KdNode {
    std::vector {
      photonHit.position.x,
      photonHit.position.y,
      photonHit.position.z
    },
    photonHit
  };
  if (isCausticMode) {
    _caustic_hits.push_back(photonHit);

    _caustic_nodes.push_back(node);
  } else {
    _hits.push_back(photonHit);

    _nodes.push_back(node);
  }
}