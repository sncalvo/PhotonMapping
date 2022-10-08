#include "Mesh.hpp"

#include <iostream>

unsigned int lastGeometryID = 0;

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material material, RTCDevice device) {
  this->vertices = vertices;
  this->indices = indices;

  _setupMesh(device, material);
}

Mesh::Mesh(const RTCGeometryType type, RTCDevice device, glm::vec4 transform, Material material) {
  _setupPrimitive(type, device, transform, material);
}

std::pair<unsigned int, Material> Mesh::getMaterialPair() const {
  // NOTE: THIS IS NOT MULTI THREAD SAFE
  // THERE IS NO DOC ON HOW GEOMETRY ID IS ASSIGNED. BUT THERE IS SOME EXAMPLE WHERE THEY JUST USE CONSECUTIVE NUMBERS
  // https://github.com/embree/embree/blob/master/tutorials/user_geometry/user_geometry_device.cpp
  return { lastGeometryID++, _material };
}

void Mesh::_setupMesh(RTCDevice device, Material material) {
  _geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
  vertexBuffer = (float*)rtcSetNewGeometryBuffer(_geometry,
                                                 RTC_BUFFER_TYPE_VERTEX,
                                                 0,
                                                 RTC_FORMAT_FLOAT3,
                                                 3 * sizeof(float),
                                                 vertices.size());

  indexBuffer = (unsigned*)rtcSetNewGeometryBuffer(_geometry,
                                                   RTC_BUFFER_TYPE_INDEX,
                                                   0,
                                                   RTC_FORMAT_UINT3,
                                                   1 * sizeof(unsigned),
                                                   indices.size());

  for (unsigned int i = 0; i < vertices.size(); ++i) {
    vertexBuffer[i * 3] = vertices[i].Position.x;
    vertexBuffer[i * 3 + 1] = vertices[i].Position.y;
    vertexBuffer[i * 3 + 2] = vertices[i].Position.z;
  }

  for (unsigned int i = 0; i < indices.size(); ++i) {
    indexBuffer[i] = indices[i];
  }

  // TODO: Change this to the actual texture
  _material = material;
}

void Mesh::_setupPrimitive(const RTCGeometryType type, RTCDevice device, glm::vec4 transform, Material material) {
  _geometry = rtcNewGeometry(device, type);

  auto buffer = (float*)rtcSetNewGeometryBuffer(_geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT4, sizeof(float) * 4, 1);

  buffer[0] = transform.x;
  buffer[1] = transform.y;
  buffer[2] = transform.z;
  buffer[3] = transform.w;

  _material = material;
}

void Mesh::commit(RTCScene scene) {
  rtcCommitGeometry(_geometry);

  rtcAttachGeometry(scene, _geometry);

  rtcReleaseGeometry(_geometry);
}
