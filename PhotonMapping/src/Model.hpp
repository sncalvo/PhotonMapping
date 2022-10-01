#pragma once

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <embree3/rtcore.h>

#include <vector>
#include <iostream>

#include "Mesh.hpp"

class Model {
public:
    Model(const std::string& objectPath, RTCDevice device);
    
    void addToEmbreeScene(RTCScene scene) const;

private:
    RTCDevice _device;
    std::vector<Mesh> _meshes;
    
    void _commit(RTCScene scene) const;
    
    void _loadModel(std::string const &path);
    
    glm::mat4 _searchLongerAABBTransformation(const aiScene *scene) const;
    
    /*
     processes a node in a recursive fashion.
     Processes each individual mesh located at the node and repeats this process
     on its children nodes (if any).
     */
    void _processNode(aiNode *node, const aiScene *scene, glm::mat4 transformation);
    
    Mesh _processMesh(aiMesh *mesh, const aiScene *scene, glm::mat4 transformation);
};
