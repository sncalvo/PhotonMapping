#include "Renderer.hpp"

#include <iostream>
#include <glm/gtx/norm.hpp>

#include "EmbreeWrapper.hpp"
#include "Constants.hpp"

void Renderer::setScene(std::shared_ptr<Scene> scene) {
  _scene = scene;
}

void Renderer::setTree(std::shared_ptr<Kdtree::KdTree> tree) {
  _tree = tree;
}

void Renderer::setCausticsTree(std::shared_ptr<Kdtree::KdTree> tree) {
  _caustics_tree = tree;
}

glm::vec3 Renderer::renderPixel(
  uint_fast32_t x,
  uint_fast32_t y,
  uint_fast32_t width,
  uint_fast32_t height,
  Color3f* pmColor
) {
  // TODO: Multiple samples per pixel
  return _renderPixelSample(x, y, width, height, pmColor);
}

glm::vec3 Renderer::_renderPixelSample(
  uint_fast32_t x,
  uint_fast32_t y,
  uint_fast32_t width,
  uint_fast32_t height,
  Color3f* pmColor
) {
  auto camera = _scene->getCamera();
  auto direction = camera->pixelRayDirection(x, y, width, height);

  return _calculateColor(camera->origin, direction, INT_CONSTANTS[MAX_DEPTH], pmColor, false);
}

float discDistanceFactor(glm::vec3 photon_position, Intersection &intersection, float delta, bool gaussian_mode = true) {
  auto diff = glm::normalize(photon_position - intersection.position);
  bool in_disc_plane = glm::abs(glm::dot(diff, intersection.normal)) <= 100.f * FLOAT_CONSTANTS[EPSILON];
  bool points_close = glm::distance2(photon_position, intersection.position) <= 100.f * FLOAT_CONSTANTS[EPSILON];

  if (points_close || in_disc_plane) {
    if (gaussian_mode) {
      auto exp_factor = (2.f * delta * delta);
      auto filter_factor = 1.f / (exp_factor * PI);

      auto x = intersection.position.x - photon_position.x;
      auto y = intersection.position.y - photon_position.y;
      auto z = intersection.position.z - photon_position.z;
      auto exp = (x * x + y * y + z * z) / exp_factor;
      return filter_factor * glm::exp(-exp);
    } else {
      return 1 + glm::distance(photon_position, intersection.position);
    }
  } else {
    return 0.f;
  }
}

Color3f Renderer::_calculateColor(glm::vec3 origin, glm::vec3 direction, unsigned int depth, Color3f* pmColor, bool in) {
  auto result = _castRay(origin, direction);

  if (!result.has_value()) {
    return _scene->ambient;
  }

  auto intersection = result.value();

  if (intersection.material.emmisive) {
    return Color3f { 1.f };
  }

  Color3f diffuseColor { 0.f };
  Color3f specularColor { 0.f };
  Color3f transparentColor { 0.f };

  if (intersection.material.diffuse > 0.f) {
    diffuseColor = _renderDiffuse(intersection);
  }

  if (intersection.material.reflection > 0.f && !in) {
    specularColor = _renderSpecular(intersection, depth, pmColor, in);
  }

  if (intersection.material.transparency > 0.f) {
    transparentColor = _renderTransparent(intersection, depth, pmColor, in);
  }

  std::vector<float> point{ intersection.position.x, intersection.position.y, intersection.position.z };
  Kdtree::KdNodeVector* caustic_neighbors = new std::vector<Kdtree::KdNode>();
  _caustics_tree->range_nearest_neighbors(point, FLOAT_CONSTANTS[MAX_PHOTON_SAMPLING_DISTANCE] / 2.f, caustic_neighbors);

  glm::vec3 caustics{ 0.f };
  for (auto neighbor : *caustic_neighbors) {
    auto rho = intersection.material.diffuseColor();
    auto weight = discDistanceFactor(neighbor.data.position, intersection, FLOAT_CONSTANTS[DELTA] / 3.f);
    caustics += neighbor.data.power * weight * rho;
  }

  delete caustic_neighbors;

  auto rayTracing = diffuseColor + specularColor + transparentColor;
  auto indirectIllumination = _computeRadianceWithPhotonMap(intersection);

  if (indirectIllumination.x < 0.f || indirectIllumination.y < 0.f || indirectIllumination.z < 0.f) {
    std::cout << indirectIllumination.x << ", " << indirectIllumination.y << ", " << indirectIllumination.z << std::endl;
  }
  if (indirectIllumination.x >= 1000.f || indirectIllumination.y >= 1000.f || indirectIllumination.z >= 1000.f) {
    std::cout << indirectIllumination.x << ", " << indirectIllumination.y << ", " << indirectIllumination.z << std::endl;
  }
  
  pmColor[0] += indirectIllumination;
  pmColor[1] += caustics;
  if (depth == INT_CONSTANTS[MAX_DEPTH]) {
    pmColor[2] += rayTracing;
  }
  return rayTracing + indirectIllumination + caustics;
}

Color3f Renderer::_computeRadianceWithPhotonMap(Intersection &intersection) {
  glm::vec3 indirectIllumination { 0.f };

  std::vector<float> point{ intersection.position.x, intersection.position.y, intersection.position.z };
  Kdtree::KdNodeVector* neighbors = new std::vector<Kdtree::KdNode>();
  _tree->range_nearest_neighbors(point, FLOAT_CONSTANTS[MAX_PHOTON_SAMPLING_DISTANCE], neighbors);

  for (auto neighbor : *neighbors) {
      auto rho = intersection.material.diffuseColor();
      auto distanceFactor = discDistanceFactor(neighbor.data.position, intersection, FLOAT_CONSTANTS[DELTA]);
      auto power = neighbor.data.power;
      if(isnan(power.x) || isnan(power.y) || isnan(power.z)) {
        continue;
      }
      indirectIllumination += distanceFactor * rho * power;
  }
 
  delete neighbors;
  
  return indirectIllumination;
}

std::optional<Intersection> Renderer::_castRay(glm::vec3 origin, glm::vec3 direction) {
  return intersectRay(origin, direction, _scene);
}

Color3f Renderer::_renderDiffuse(Intersection &intersection) {
  Color3f color{ 0.f };
  
  for (const auto light : _scene->getLights()) {
    color += light->intensityFrom(intersection, _scene->scene);
  }

  return color * intersection.material.diffuse;
}

Color3f Renderer::_renderSpecular(Intersection &intersection, unsigned int depth, Color3f* pmColor, bool in) {
  if (depth == 0) {
    return Color3f {0.f};
  }
  // Not sure why the -1000.0f. This was taken from the last's year ray tracing
  auto origin = intersection.position;
  auto reflectionDirection = glm::normalize(glm::reflect(intersection.direction, intersection.normal));

  auto color = _calculateColor(origin, reflectionDirection, depth - 1, pmColor, in) * intersection.material.reflection;

  return color;
}

unsigned int invertedNormalCount = 0;
unsigned int nonInvertedNormalCount = 0;

Color3f Renderer::_renderTransparent(Intersection &intersection, unsigned int depth, Color3f* pmColor, bool in) {
  if (depth == 0) {
    return Color3f { 0.f };
  }

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
  glm::vec3 refractionDirection; 
  bool newIn = in;
  if (discriminant < 0) {
    refractionDirection = glm::normalize(glm::reflect(intersection.direction, intersection.normal));
  }
  else {
    refractionDirection = nuIt * intersection.direction + (Ci * nuIt - sqrtf(discriminant)) * normal;
    newIn = !in;
  }

  glm::vec3 refractionPosition = intersection.position + FLOAT_CONSTANTS[EPSILON] * refractionDirection;

  Color3f color{ 0.f };

  Color3f color_factor = glm::vec3{1.f};
  if (!in) {
    color_factor = intersection.material.transparencyColor();
  }

  color += _calculateColor(
    refractionPosition,
    refractionDirection, depth - 1, pmColor, newIn
  );

//  if (refractionRatio * sinTheta <= 1.f) {
//    // shorturl.at/cDGZ1
//    glm::vec3 refractionNormal = intersection.normal;
//
//    auto refractionPerpendicular = refractionRatio * (intersection.direction + cosTheta * refractionNormal);
//    auto refractionParallel = -glm::sqrt(
//      std::abs(1.f - glm::dot(refractionPerpendicular, refractionPerpendicular))
//    ) * refractionNormal;
//
//    auto refractionDirection = refractionPerpendicular + refractionParallel;
//    color += _calculateColor(
//      intersection.position + glm::vec3(FLOAT_CONSTANTS[EPSILON]) * intersection.direction,
//      refractionDirection, depth - 1
//    ) * intersection.material.transparency;
//  }

  return color * color_factor;
}
