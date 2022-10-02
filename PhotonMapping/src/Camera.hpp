#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    glm::vec3 horizontal, vertical, lowerLeftCorner, origin{0.f, 0.f, -1.f};
    
    /// Calculates ray direction for given pixel and the width and height for the image
    /// - Parameters:
    ///   - column: x coordinate for the pixel in image that is being rendered
    ///   - row: y coordinate for the pixel in image that is being rendered
    ///   - width: width of the image (i.e. 1920 in an image of 1920x1080)
    ///   - height: height of the image (i.e. 1080 in an image of 1920x1080)
    inline auto pixelRayDirection(uint_fast32_t column, uint_fast32_t row, uint_fast32_t width, uint_fast32_t height) {
        // Take u to range of [0-1]
        auto u = float(column) / float(width - 1);
        // Take v to range of [0-1]
        auto v = float(row) / float(height - 1);
        
        // Given the lower left corner direction at the frame, get the direction of the ray by summing the vertical and
        // horizontal direction
        return lowerLeftCorner + u * horizontal + v * vertical - origin;
    }
    
    /// Constructs a camera using aspect ratio, height modifier and focal length
    /// - Parameters:
    ///   - aspectRatio: aspect ratio for the camera
    ///   - height: height modifier
    ///   - focalLength: focal length for the camera (how much zoom there is, causing the image to be distorted)
    Camera(float aspectRatio, float height, float focalLength);
    
    /// Constructs a camera using aspect ratio and focal length
    /// - Parameters:
    ///   - aspectRatio: aspect ratio for the camera
    ///   - focalLength: focal length for the camera (how much zoom there is, causing the image to be distorted)
    Camera(float aspectRatio, float focalLength) : Camera(aspectRatio, 2.f, focalLength) {};

private:
    glm::vec3 _calculateLowerLeftCorner(glm::vec3 origin, glm::vec3 horizontal, glm::vec3 vertical, float focalLength);

private:
    const float _viewportHeight;
    const float _viewportWidth;

    const float _focalLength = 1.f;
    const glm::vec3 _origin = glm::vec3{0.f};
};
