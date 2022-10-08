#pragma once

#include <vector>
#include <iostream>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <embree3/rtcore.h>

#include "Mesh.hpp"

/// Model that represents a list of meshes used in the scene for ray tracing and path tracing
class Model {
public:
  /// Contructs the model from the model file and saves the device for later use and generates meshes for it with their geometries using the device
  /// - Parameters:
  ///   - objectPath: path to object that will be read
  ///   - material: material for meshes inside model
  ///   - device: device used to generate the geometries
  Model(const std::string& objectPath, Material material, RTCDevice device);

  /// Creates a model with a embree primitive
  /// - Parameters:
  ///    - geometryType: embree primitive type. Only supports spheres for now
  ///    - material: material for mesh of primitive
  Model(const RTCGeometryType geometryType, Material material, RTCDevice device, glm::vec4 transform);

  /// Commits all meshes in the scene for ray tracing
  /// - Parameter scene: scene used to commit meshes
  void commit(RTCScene scene) const;

  /// Returns material map of geometry id and material for easy search
  /// - Parameter scene: scene used to commit meshes
  std::unordered_map<unsigned int, Material> getMaterialsMap() const;

private:
  RTCDevice _device;
  std::vector<Mesh> _meshes;
  std::unordered_map<unsigned int, Material> _materials;

  void _loadModel(std::string const &path, Material material);
  void _loadPrimitive(const RTCGeometryType geometryType, glm::vec4 transform, Material material);

  void _processNode(aiNode *node, const aiScene *scene, Material material);

  Mesh _processMesh(aiMesh *mesh, const aiScene *scene, Material material);
};
