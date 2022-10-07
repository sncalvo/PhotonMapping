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
  RTCDevice device = initializeDevice();
  auto scene = std::make_shared<Scene>(device);

  auto floor = std::make_shared<Model>("./assets/floor.obj", device);
  auto ball = std::make_shared<Model>("./assets/ball.obj", device);
  auto cube = std::make_shared<Model>("./assets/cube.obj", device);
  auto sphere = std::make_shared<Model>(RTC_GEOMETRY_TYPE_SPHERE_POINT, device, glm::vec4 { 2.0f, 2.0f, 5.0f, 0.5f });
  
  scene->addModel(floor);
  scene->addModel(ball);
  scene->addModel(cube);
  scene->addModel(sphere);
  scene->commit();
  scene->addLight(Light {
    glm::vec3 {-2.f, 2.0f, 1.f},
    glm::vec3 {1.f, 1.f, 1.f},
    1.f,
    0.2f,
    0.09f,
    0.0f,
  });

  auto image = new Image(1920, 1080);
  const auto aspectRatio = (float)image->width / (float)image->height;
  auto camera = std::make_shared<Camera>(aspectRatio, 1.f);

  scene->setCamera(camera);

  Renderer renderer;

  renderer.setScene(scene);

  for (unsigned int x = 0; x < image->width; ++x) {
    for (unsigned int y = 0; y < image->height; ++y) {
      Color3f color = renderer.renderPixel(x, y, image->width, image->height);

      image->writePixel(x, y, Color { color });
    }
  }

  image->save("test.png");

  rtcReleaseDevice(device);

  return 0;
}
