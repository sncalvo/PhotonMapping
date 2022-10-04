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
