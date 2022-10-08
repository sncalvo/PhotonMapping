#include "PhotonMapper.hpp"

#include <glm/gtc/random.hpp>

#include "EmbreeWrapper.hpp"
#include "Constants.hpp"
#include "Image.hpp"

constexpr unsigned int PHOTON_LIMIT = 1000000;

PhotonMapper::PhotonMapper() {
  
}

auto random01() {
  return (float) rand() / RAND_MAX;
}

auto random() {
  return random01() * 2.f - 1;
}

glm::vec3 randomNormalizedVector() {
  auto x = random();
  auto y = random();
  auto z = random();

  while (x*x+y*y+z*z > 1) {
    x = random();
    y = random();
    z = random();
  }

  return glm::normalize(glm::vec3(x,y,z));
}

void PhotonMapper::makeMap(const Camera& camera) const {
  auto image = Image(1920, 1080);

  for (auto hit : _hits) {
    auto cameraPointPosition = camera.getProjectionMatrix() * camera.getViewMatrix() * glm::vec4(hit.position, 1.f);

    auto u = image.width - ((cameraPointPosition.x * image.width) / (2.f * cameraPointPosition.w) + image.width / 2.f);
    auto v = (cameraPointPosition.y * image.height) / (2.f * cameraPointPosition.w) + image.height / 2.f;

    if (u >= image.width || u < 0 || v >= image.height || v < 0) {
      continue;
    }

    image.writePixel((unsigned int)u, (unsigned int)v, Color(glm::vec3{ 1.f }));
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
      auto position = light.position;

      _shootPhoton(position, direction, light, 10);
    }
  }
}

void PhotonMapper::_shootPhoton(const glm::vec3 origin, const glm::vec3 direction, const Light light, int depth) {
  auto rayIntersection = intersectRay(origin, direction, _scene);

  if (!rayIntersection.has_value()) {
    return;
  }
  auto intersection = rayIntersection.value();
  auto photonHit = PhotonHit{
    intersection.position,
    intersection.direction,
    light.intensity
  };

  if (depth == 0) {
    return;
  }

  auto diffuseThreshold = intersection.material.diffuse;
  auto reflectionThreshold = intersection.material.reflection + diffuseThreshold;
  auto transparentThreshold = intersection.material.transparency + reflectionThreshold;

  auto randomSample = random01();

  if (randomSample <= diffuseThreshold) {
    _hits.push_back(photonHit);

    auto reflectionDirection = randomNormalizedVector();

    if (glm::dot(reflectionDirection, intersection.normal) < 0) {
      reflectionDirection = -reflectionDirection;
    }

    auto reflectionPosition = intersection.position + EPSILON * reflectionDirection;

    _shootPhoton(reflectionPosition, reflectionDirection, light, depth - 1);
  } else if (randomSample <= reflectionThreshold) {
    auto reflectionDirection = glm::reflect(intersection.direction, intersection.normal);

    auto reflectionPosition = intersection.position + EPSILON * reflectionDirection;

    _shootPhoton(reflectionPosition, reflectionDirection, light, depth - 1);
  } else if (randomSample <= transparentThreshold) {
    auto refractionDirection = intersection.direction;

    auto refractionPosition = intersection.position + EPSILON * refractionDirection;

    _shootPhoton(refractionPosition, refractionDirection, light, depth - 1);
  } else {
    _hits.push_back(photonHit);
  }
}