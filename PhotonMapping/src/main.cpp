#include <memory>
#include <iostream>

#include <embree3/rtcore.h>

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

bool castRay(RTCScene scene, Vector vector)
{
    /*
     * The intersect context can be used to set intersection
     * filters or flags, and it also contains the instance ID stack
     * used in multi-level instancing.
     */
    struct RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    
    /*
     * The ray hit structure holds both the ray and the hit.
     * The user must initialize it properly -- see API documentation
     * for rtcIntersect1() for details.
     */
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
    
    /*
     * There are multiple variants of rtcIntersect. This one
     * intersects a single ray with the scene.
     */
    rtcIntersect1(scene, &context, &rayhit);

    return rayhit;
}

glm::vec3 renderPixelSample(
  float x,
  float y,
  const Camera &camera,
  const Image& image,
  const Scene& scene
)
{
  auto radiance = glm::vec3(0.f);

  // Build vector coming from x, y
  auto rayDirection = camera->pixelRayDirection(x, y, image->width, image->height);

  Vector ray{
      camera->origin.x, camera->origin.y, camera->origin.z,
      rayDirection.x, rayDirection.y, rayDirection.z
  };

  auto hit = castRay(scene->scene, ray);

  /* Note how geomID and primID identify the geometry we just hit.
   * We could use them here to interpolate geometry information,
   * compute shading, etc.
   * Since there is only a single triangle in this scene, we will
   * get geomID=0 / primID=0 for all hits.
   * There is also instID, used for instancing. See
   * the instancing tutorials for more information */

  // If nothing hit, use lights
  if (hit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
  {
      for (unsigned int i = 0; i < scene->getNumLights(); i++)
      {
        const Light* light = scene->getLight(i);
        radiance = radiance + light.value;
      }
  }
  
  return false;
}

glm::vec3 renderPixel(
  float x,
  float y,
  const Camera &camera,
  const Image& image,
  const Scene& scene
)
{
  // TODO: Add more than one sample per pixel

  return renderPixelSample(x, y, camera);
}

int main()
{
    RTCDevice device = initializeDevice();
    auto scene = std::make_unique<Scene>(device);

    auto model = std::make_shared<Model>("./assets/cubito.obj", device);
    
    scene->addModel(model);
    scene->commit();
    
    auto image = new Image(1920, 1080);
    const auto aspectRatio = (float)image->width / (float)image->height;
    auto camera = std::make_unique<Camera>(aspectRatio, 1.f);
    
    for (unsigned int x = 0; x < image->width; ++x) {
        for (unsigned int y = 0; y < image->height; ++y) {
            renderPixel(x, y, camera, image, scene);

            if (hit) {
                image->writePixel(x, y, Color { 125, 125 });
            } else {
                image->writePixel(x, y, Color { 0, 0, 125 });
            }
        }
    }

    image->save("test.png");

    rtcReleaseDevice(device);
    
    return 0;
}
