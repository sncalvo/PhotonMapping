#include "embree3/rtcore.h"
#include <stdio.h>
#include <math.h>
#include <limits>
#include <stdio.h>
#include <filesystem>

#include <iostream>

#include "assimp/Importer.hpp"      // C++ importer interface
#include "assimp/scene.h"           // Output data structure
#include "assimp/postprocess.h"     // Post processing flags

#include "Model.hpp"
#include "Scene.hpp"
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

/*
 * Cast a single ray with origin (ox, oy, oz) and direction
 * (dx, dy, dz).
 */
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
    
    if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
    {
        /* Note how geomID and primID identify the geometry we just hit.
         * We could use them here to interpolate geometry information,
         * compute shading, etc.
         * Since there is only a single triangle in this scene, we will
         * get geomID=0 / primID=0 for all hits.
         * There is also instID, used for instancing. See
         * the instancing tutorials for more information */
        
        return true;
    }
    
    return false;
}

int main()
{
    printf("BEGIN.\n");
    
    std::cout << std::filesystem::current_path() << std::endl;
    
    /* Initialization. All of this may fail, but we will be notified by
     * our errorFunction. */
    RTCDevice device = initializeDevice();
    Scene* scene = new Scene(device);
    
    Model* model = new Model("./assets/cubito.obj", device);
    
    scene->addModel(model);
    scene->commit();
    
    auto image = new Image(256, 256);
    
    for (unsigned int x = 0; x < image->width; ++x) {
        float ox = (float)x / (float)image->width;
        
        for (unsigned int y = 0; y < image->height; ++y) {
            // Build vector coming from x, y
            float oy = (float)y / (float)image->height;
            
            // Intersect vector
            Vector ray{
                ox, oy, -1,
                0., 0., 1.
            };
            
            auto hit = castRay(scene->scene, ray);
            
            // Write vector result
            if (hit) {
                image->writePixel(x, y, Color { 125, 125 });
            } else {
                image->writePixel(x, y, Color { 0, 0, 125 });
            }
        }
    }

    image->save("test.png");

    delete scene;
    
    rtcReleaseDevice(device);
    
    return 0;
}
