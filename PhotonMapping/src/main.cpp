#include <memory>
#include <iostream>

#include <embree3/rtcore.h>
#include <math.h>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/constants.hpp>

#include "Constants.hpp"
#include "Model.hpp"
#include "Scene.hpp"
#include "Camera.hpp"
#include "Image.hpp"
#include "Vector.hpp"
#include "Renderer.hpp"
#include "PhotonMapper.hpp"
#include "SceneBuilder.hpp"

#include "Utils.hpp"

constexpr unsigned int numberOfPhotons = 10000;
constexpr auto loadTree = true;

constexpr auto photonsTreeFilename = "photonsTree";
constexpr auto causticsTreeFilename = "causticsTree";

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

  SceneBuilder sceneBuilder = SceneBuilder(device);
  std::shared_ptr<Scene> scene = sceneBuilder.createScene();

  auto image = new Image(INT_CONSTANTS[WIDTH], INT_CONSTANTS[HEIGHT]);
  auto globalPMImage = new Image(INT_CONSTANTS[WIDTH], INT_CONSTANTS[HEIGHT]);
  auto causticsImage = new Image(INT_CONSTANTS[WIDTH], INT_CONSTANTS[HEIGHT]);
  const auto aspectRatio = (float)image->width / (float)image->height;
  auto camera = std::make_shared<Camera>(aspectRatio, 1.f);

  Renderer renderer;

  renderer.setScene(scene);

  // Generate Photon image
  PhotonMapper photonMapper = PhotonMapper();

  photonMapper.useScene(scene);

  if (loadTree) {
    photonMapper.initializeTreeFromFile(photonsTreeFilename, causticsTreeFilename);
  } else {
    photonMapper.makeGlobalPhotonMap(PhotonMap::Global);
    photonMapper.makeCausticsPhotonMap(PhotonMap::Global);

    photonMapper.saveTreeToFile(photonsTreeFilename, causticsTreeFilename);
  }

  photonMapper.makeMap(*scene->getCamera());

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
