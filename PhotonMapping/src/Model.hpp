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
  ///   - device: device used to generate the geometries
  Model(const std::string& objectPath, RTCDevice device);

  /// Commits all meshes in the scene for ray tracing
  /// - Parameter scene: scene used to commit meshes
  void commit(RTCScene scene) const;

  /// Returns material map of geometry id and material for easy search
  /// - Parameter scene: scene used to commit meshes
  std::unordered_map<RTCGeometry, Material> getMaterialsMap() const;

private:
  RTCDevice _device;
  std::vector<Mesh> _meshes;
  std::unordered_map<RTCGeometry, Material> _materials;

  void _loadModel(std::string const &path);

  void _processNode(aiNode *node, const aiScene *scene);

  Mesh _processMesh(aiMesh *mesh, const aiScene *scene);
};
