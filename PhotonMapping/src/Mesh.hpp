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
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, RTCDevice device);

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
  void _setupMesh(RTCDevice device);
};
