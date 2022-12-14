#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <embree3/rtcore.h>

#include <string>
#include <vector>

#include "Material.hpp"

struct Vertex {
    // position
  glm::vec3 Position;
};

/// Mesh representing list of vertices and indices used to then generate geometries in the embree scene
class Mesh {
public:
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  /// Constructs a mesh from verticies and indices. Also needs a device to create the geometries for embree.
  /// - Parameters:
  ///   - vertices: Vector of vertices for the mesh
  ///   - indices: Vector of indices for the mesh
  ///   - device: device to generate geometries
  ///   - material: material for mesh
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material material, RTCDevice device);

  Mesh(const RTCGeometryType type, RTCDevice device, glm::vec4 transform, Material material);

  Mesh(Material material, glm::vec3 corner, glm::vec3 uvec, glm::vec3 vvec, RTCDevice);

  /// Commits geometries to the ray tracing scene
  /// - Parameter scene: ray tracing scene that will contain the geometries
  void commit(RTCScene scene);

  /// Returns pair of material and geometry id allowing to add it to maps
  std::pair<unsigned int, Material> getMaterialPair() const;

private:
  /// render data
  float *vertexBuffer;
  unsigned int *indexBuffer;
  RTCGeometry _geometry;
  Material _material;

  /// initializes all the buffer objects/arrays
  void _setupMesh(RTCDevice device, Material material);
  void _setupPrimitive(const RTCGeometryType type, RTCDevice device, glm::vec4 transform, Material material);
  void _setupQuad(Material material, glm::vec3 corner, glm::vec3 uvec, glm::vec3 vvec, RTCDevice device);
};
