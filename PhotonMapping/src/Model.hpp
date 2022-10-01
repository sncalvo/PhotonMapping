#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <embree3/rtcore.h>

#include <vector>
#include <iostream>

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

private:
    RTCDevice _device;
    std::vector<Mesh> _meshes;

    void _loadModel(std::string const &path);

    glm::mat4 _searchLongerAABBTransformation(const aiScene *scene) const;

    void _processNode(aiNode *node, const aiScene *scene, glm::mat4 transformation);

    Mesh _processMesh(aiMesh *mesh, const aiScene *scene, glm::mat4 transformation);
};
