#include "Mesh.hpp"

#include <iostream>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, RTCDevice device) {
    this->vertices = vertices;
    this->indices = indices;
    
    setupMesh(device);
}

void Mesh::setupMesh(RTCDevice device) {
    _geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
    vertexBuffer = (float*)rtcSetNewGeometryBuffer(_geometry,
                                                   RTC_BUFFER_TYPE_VERTEX,
                                                   0,
                                                   RTC_FORMAT_FLOAT3,
                                                   3 * sizeof(float),
                                                   vertices.size());
    
    std::cout << "size:" << vertices.size() << std::endl;
    
    indexBuffer = (unsigned*)rtcSetNewGeometryBuffer(_geometry,
                                                     RTC_BUFFER_TYPE_INDEX,
                                                     0,
                                                     RTC_FORMAT_UINT3,
                                                     1 * sizeof(unsigned),
                                                     indices.size());
    
    for (unsigned int i = 0; i < vertices.size(); ++i) {
        vertexBuffer[i * 3] = vertices[i].Position.x;
        vertexBuffer[i * 3 + 1] = vertices[i].Position.y;
        vertexBuffer[i * 3 + 2] = vertices[i].Position.z + 2;
    }

    for (unsigned int i = 0; i < indices.size(); ++i) {
        indexBuffer[i] = indices[i];
    }
}

void Mesh::commit(RTCScene scene) {
    rtcCommitGeometry(_geometry);
    
    rtcAttachGeometry(scene, _geometry);
    
    rtcReleaseGeometry(_geometry);
}
