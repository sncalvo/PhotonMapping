#include <memory>
#include <iostream>

#include <embree3/rtcore.h>
#include <math.h>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/constants.hpp>

#include "Model.hpp"
#include "Scene.hpp"
#include "Camera.hpp"
#include "Image.hpp"
#include "Vector.hpp"

void errorFunction(void* userPtr, enum RTCError error, const char* str)
{
  printf("error %d: %s\n", error, str);
}

RTCDevice initializeDevice()
{
  RTCDevice device = rtcNewDevice(NULL);

  if (!device)
    printf("error %d: cannot create device\n", rtcGetDeviceError(NULL));

  rtcSetDeviceErrorFunction(device, errorFunction, NULL);
  return device;
}

auto castRay(const Scene& scene, Vector vector) {
  struct RTCIntersectContext context;
  rtcInitIntersectContext(&context);

  struct RTCRayHit rayhit;
  rayhit.ray.org_x = vector.ox;
  rayhit.ray.org_y = vector.oy;
  rayhit.ray.org_z = vector.oz;
  rayhit.ray.dir_x = vector.dx;
  rayhit.ray.dir_y = vector.dy;
  rayhit.ray.dir_z = vector.dz;
  rayhit.ray.tnear = 0;
  rayhit.ray.tfar = std::numeric_limits<float>::infinity();
  rayhit.ray.mask = -1;
  rayhit.ray.flags = 0;
  rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
  rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

  rtcIntersect1(scene.scene, &context, &rayhit);

  return rayhit;
}

float attenuation(float distance, const Light& light)
{
  return 1.f / (light.constantDecay + light.linearDecay * distance + light.quadraticDecay * glm::pow(distance, 2.f));
}

auto renderPixelSample(
  float x,
  float y,
  const Camera& camera,
  const Image& image,
  Scene& scene
) {
  // Build vector coming from x, y
  auto rayDirection = camera.pixelRayDirection(x, y, image.width, image.height);

  Vector ray{
    camera.origin.x, camera.origin.y, camera.origin.z,
    rayDirection.x, rayDirection.y, rayDirection.z
  };

  auto rayHit = castRay(scene, ray);

  if (rayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
    return glm::vec3{0.f};
  }

    // This should be color of current element
  glm::vec3 color{.0f};

    // We hit something, for know we assume diffuse and check if light is occluded
  for (const auto light : scene.getLights()) {
    auto rayHitOrigin = glm::vec3{
      rayHit.ray.org_x + rayHit.ray.dir_x * rayHit.ray.tfar,
      rayHit.ray.org_y + rayHit.ray.dir_y * rayHit.ray.tfar,
      rayHit.ray.org_z + rayHit.ray.dir_z * rayHit.ray.tfar
    };

    auto directionToLight = light.position - rayHitOrigin;

    // TODO: Mode this to vector
    struct RTCRayHit shadowRayHit;
    shadowRayHit.ray.org_x = rayHitOrigin.x;
    shadowRayHit.ray.org_y = rayHitOrigin.y;
    shadowRayHit.ray.org_z = rayHitOrigin.z;
    shadowRayHit.ray.dir_x = directionToLight.x;
    shadowRayHit.ray.dir_y = directionToLight.y;
    shadowRayHit.ray.dir_z = directionToLight.z;
    shadowRayHit.ray.tnear = std::numeric_limits<float>::epsilon();
    shadowRayHit.ray.tfar = std::numeric_limits<float>::infinity();
    shadowRayHit.ray.mask = -1;
    shadowRayHit.ray.flags = 0;
    shadowRayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    shadowRayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

    struct RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    rtcOccluded1(scene.scene, &context, &shadowRayHit.ray);

      // For some reason, >= 0 means we reached light. tfar = -inf if object is occluded
    if (shadowRayHit.ray.tfar >= 0.f) {
      auto normal = glm::vec3{ rayHit.hit.Ng_x, rayHit.hit.Ng_y, rayHit.hit.Ng_z };
      auto diffuse = glm::vec3{0.4f} * light.color * std::max(glm::dot(normal, directionToLight), 0.f);

      auto distanceToLight = glm::l2Norm(light.position, rayHitOrigin);
      auto lightAttenuation = attenuation(distanceToLight, light);

      color += lightAttenuation * light.intensity * diffuse;
    }
  }
  
  return color;
}

auto renderPixel(
                 float x,
                 float y,
                 const Camera &camera,
                 const Image& image,
                 Scene& scene
                 ) {
    // TODO: Add more than one sample per pixel

  return renderPixelSample(x, y, camera, image, scene);
}

int main()
{
  RTCDevice device = initializeDevice();
  auto scene = std::make_unique<Scene>(device);

  auto model = std::make_shared<Model>("./assets/cubito.obj", device);

  scene->addModel(model);
  scene->commit();
  scene->addLight(Light {
    glm::vec3 {0.f, 0.5f, 0.f},
    glm::vec3 {1.f, 1.f, 1.f},
    1.f,
    0.f,  // 1.f,
    0.f,  // 0.09f,
    0.f  // 0.032f,
  });

  auto image = new Image(1920, 1080);
  const auto aspectRatio = (float)image->width / (float)image->height;
  auto camera = std::make_unique<Camera>(aspectRatio, 1.f);

  for (unsigned int x = 0; x < image->width; ++x) {
    for (unsigned int y = 0; y < image->height; ++y) {
      glm::vec3 color = renderPixel(x, y, *camera, *image, *scene);

      image->writePixel(x, y, Color { color });
    }
  }

  image->save("test.png");

  rtcReleaseDevice(device);

  return 0;
}
