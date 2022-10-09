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
  auto sphere = std::make_shared<Model>(RTC_GEOMETRY_TYPE_SPHERE_POINT, Material { glm::vec3 { 0.8f, 0.8f, 0.8f }, 0.0f, 0.0f, 0.9f, 1.5f }, device, glm::vec4 { -0.5f, -2.0f, 7.0f, 1.0f });
  auto sphere2 = std::make_shared<Model>(RTC_GEOMETRY_TYPE_SPHERE_POINT, Material { glm::vec3 { 0.8f, 0.8f, 0.8f }, 0.0f, 0.9f, 0.f, }, device, glm::vec4 { 2.0f, -2.0f, 7.0f, 1.0f });
  auto backWall = std::make_shared<Model>("./assets/backwall.obj", Material { glm::vec3 { 1.f, 1.f, 1.f }, 0.9f, 0.f, 0.f,  }, device);
  auto leftWall = std::make_shared<Model>("./assets/leftwall.obj", Material { glm::vec3 { 1.f, 0.f, 0.f }, 0.9f, 0.f, 0.f,  }, device);
  auto rightWall = std::make_shared<Model>("./assets/rightwall.obj", Material { glm::vec3 { 0.f, 0.f, 1.f }, 0.9f, 0.f, 0.f,  }, device);
  auto ceiling = std::make_shared<Model>("./assets/ceiling.obj", Material { glm::vec3 { 1.f, 1.f, 1.f }, 0.9f, 0.f, 0.f,  }, device);

  std::shared_ptr<Light> light = std::make_shared<AreaLight>(
    glm::vec3 {0.0f, 3.9f, 7.f},
    glm::vec3 {1.f, 1.f, 1.f},
    1.f,
    0.2f,
    0.09f,
    0.042f,
    glm::vec3{0.5, 0.f, 0.f},
    glm::vec3{0.f, 0.f, 0.5f},
    3,
    2,
    device
  );

  std::shared_ptr<Light> light2 = std::make_shared<AreaLight>(
    glm::vec3 {-3.f, 0.f, 6.f},
    glm::vec3 {1.f, 1.f, 1.f},
    1.f,
    0.2f,
    0.09f,
    0.042f,
    glm::vec3{0.0f, 0.5f, 0.f},
    glm::vec3{0.f, 0.f, 0.5},
    3,
    2,
    device
  );

  scene->addModel(floor);
/*   scene->addModel(ball1); */
/*   scene->addModel(ball2); */
  scene->addModel(sphere);
  scene->addModel(sphere2);
  scene->addModel(backWall);
  scene->addModel(leftWall);
  scene->addModel(rightWall);
  scene->addModel(ceiling);
  scene->addLight(light);
  scene->addLight(light2);
  scene->commit();

  auto image = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);
  auto globalPMImage = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);
  auto causticsImage = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);
  const auto aspectRatio = (float)image->width / (float)image->height;
  auto camera = std::make_shared<Camera>(aspectRatio, 1.f);

  scene->setCamera(camera);

  Renderer renderer;

  renderer.setScene(scene);

  // Generate Photon image
  PhotonMapper photonMapper = PhotonMapper();

  photonMapper.useScene(scene);

  photonMapper.makeGlobalPhotonMap(PhotonMap::Global);
  photonMapper.makeCausticsPhotonMap(PhotonMap::Global);
  photonMapper.makeMap(*camera);

  renderer.setTree(photonMapper.getTree());
  renderer.setCausticsTree(photonMapper.getCausticsTree());

  for (unsigned int x = 0; x < image->width; ++x) {
    for (unsigned int y = 0; y < image->height; ++y) {
      Color3f pmColor[2] = {glm::vec3(0.f), glm::vec3(0.f)};
      Color3f color = renderer.renderPixel(x, y, image->width, image->height, pmColor);

      image->writePixel(x, y, color);
      globalPMImage->writePixel(x, y, pmColor[0]);
      causticsImage->writePixel(x, y, pmColor[1]);
    }
  }

  image->save("test.png");
  globalPMImage->save("globalPM.png");
  causticsImage->save("caustics.png");

  rtcReleaseDevice(device);

  return 0;
}
