#include "./Object.hpp"

#include <iostream>

Object::Object(const std::string& objectPath) {
    Assimp::Importer importer;

    _scene = importer.ReadFile(objectPath,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    if (!_scene || _scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !_scene->mRootNode) {
        std::cout << importer.GetErrorString() << std::endl;
        throw "Object not valid";
    }

    std::string name = std::string(_scene->mMeshes[0]->mName.C_Str());
    std::cout << name << std::endl;

    // TODO: Probably process the scene to some format for us to use
}
