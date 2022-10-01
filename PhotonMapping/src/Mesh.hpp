#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <embree3/rtcore.h>

#include <string>
#include <vector>

struct Vertex {
    // position
    glm::vec3 Position;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    
    unsigned int VAO;
    
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, RTCDevice device);
    
    void commit(RTCScene scene);
    
private:
    // render data
    float *vertexBuffer;
    unsigned int *indexBuffer;
    RTCGeometry _geometry;
    
    // initializes all the buffer objects/arrays
    void setupMesh(RTCDevice device);
};
