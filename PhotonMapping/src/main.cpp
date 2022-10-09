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
#include "Renderer.hpp"
#include "PhotonMapper.hpp"

#include "Utils.hpp"

constexpr unsigned int numberOfPhotons = 10000;

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

int main()
{
  std::srand(time(0));
  RTCDevice device = initializeDevice();
  auto scene = std::make_shared<Scene>(device);

  auto floor = std::make_shared<Model>("./assets/plane.obj", Material { glm::vec3 { 1.f, 1.f, 1.f }, 0.9f, 0.f, 0.f,  }, device);
  auto ball1 = std::make_shared<Model>(RTC_GEOMETRY_TYPE_SPHERE_POINT, Material { glm::vec3 { 1.f, 0.f, 1.f }, 0.2f, 0.1f, 0.6f,  }, device, glm::vec4 { -1.0f, -1.0f, 8.0f, 2.0f });
  auto ball2 = std::make_shared<Model>(RTC_GEOMETRY_TYPE_SPHERE_POINT, Material { glm::vec3 { 1.f, 0.f, 0.f }, 0.2f, 7.f, 0.f,  }, device, glm::vec4 { 1.0f, -2.f, 9.0f, 1.0f });
  auto sphere = std::make_shared<Model>(RTC_GEOMETRY_TYPE_SPHERE_POINT, Material { glm::vec3 { 0.f, 1.f, 1.f }, 0.9f, 0.f, 0.f,  }, device, glm::vec4 { 2.0f, -2.0f, 7.0f, 1.0f });
  auto backWall = std::make_shared<Model>("./assets/backwall.obj", Material { glm::vec3 { 1.f, 1.f, 1.f }, 0.9f, 0.f, 0.f,  }, device);
  auto leftWall = std::make_shared<Model>("./assets/leftwall.obj", Material { glm::vec3 { 1.f, 0.f, 0.f }, 0.9f, 0.f, 0.f,  }, device);
  auto rightWall = std::make_shared<Model>("./assets/rightwall.obj", Material { glm::vec3 { 0.f, 0.f, 1.f }, 0.9f, 0.f, 0.f,  }, device);
  auto ceiling = std::make_shared<Model>("./assets/ceiling.obj", Material { glm::vec3 { 1.f, 1.f, 1.f }, 0.9f, 0.f, 0.f,  }, device);
  
  scene->addModel(floor);
  scene->addModel(ball1);
  scene->addModel(ball2);
  scene->addModel(sphere);
  scene->addModel(backWall);
  scene->addModel(leftWall);
  scene->addModel(rightWall);
  scene->addModel(ceiling);
  scene->commit();
  scene->addLight(Light {
    glm::vec3 {0.f, 2.f, 5.f},
    glm::vec3 {1.f, 1.f, 1.f},
    1.f,
    0.2f,
    0.09f,
    0.0f,
  });

  auto image = new Image(854, 480);
  const auto aspectRatio = (float)image->width / (float)image->height;
  auto camera = std::make_shared<Camera>(aspectRatio, 1.f);

  scene->setCamera(camera);

  Renderer renderer;

  renderer.setScene(scene);

  // Generate Photon image
  PhotonMapper photonMapper = PhotonMapper();

  photonMapper.useScene(scene);

  photonMapper.makePhotonMap(PhotonMap::Global);
  photonMapper.makeMap(*camera);

  renderer.setTree(photonMapper.getTree());

  for (unsigned int x = 0; x < image->width; ++x) {
    for (unsigned int y = 0; y < image->height; ++y) {
      Color3f color = renderer.renderPixel(x, y, image->width, image->height);

      image->writePixel(x, y, color);
    }
  }

  image->save("test.png");

  rtcReleaseDevice(device);

  return 0;
}
