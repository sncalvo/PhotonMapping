#include "./Model.hpp"

#include <iostream>

auto middlePointInAABB(glm::vec3 min, glm::vec3 max) {
    auto middle = (min + max) / 2.0f;

    return middle;
}

auto longerAxisLength(glm::vec3 min, glm::vec3 max) {
    auto diff = max - min;
    auto xLength = diff.x;
    auto yLength = diff.y;
    auto zLength = diff.z;
    
    if (xLength > yLength && xLength > zLength) {
        return xLength;
    }
    
    if (yLength > zLength) {
        return yLength;
    }
    
    return zLength;
}

Model::Model(const std::string& objectPath, RTCDevice device) : _device(device) {
    _loadModel(objectPath);
}

void Model::addToEmbreeScene(RTCScene scene) const {
    _commit(scene);
}

void Model::_commit(RTCScene scene) const  {
    for (auto mesh : _meshes) {
        mesh.commit(scene);
    }
}

void Model::_loadModel(std::string const &path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
                                             aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                             aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    
    auto transformation = _searchLongerAABBTransformation(scene);
    
    // process ASSIMP's root node recursively
    _processNode(scene->mRootNode, scene, transformation);
}

glm::mat4 Model::_searchLongerAABBTransformation(const aiScene *scene) const {
    glm::vec3 min(std::numeric_limits<float>::infinity());
    glm::vec3 max(0.f);
    
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        auto mesh = scene->mMeshes[i];
        auto aabb = mesh->mAABB;
        
        if (aabb.mMin.x < min.x) {
            min.x = aabb.mMin.x;
        }
        if (aabb.mMin.y < min.y) {
            min.y = aabb.mMin.y;
        }
        if (aabb.mMin.z < min.z) {
            min.z = aabb.mMin.z;
        }
        
        if (aabb.mMax.x > max.x) {
            max.x = aabb.mMax.x;
        }
        if (aabb.mMax.y > max.y) {
            max.y = aabb.mMax.y;
        }
        if (aabb.mMax.z > max.z) {
            max.z = aabb.mMax.z;
        }
    }
    
    std::cout << min.x << "," << min.y << "," << min.z << std::endl;
    std::cout << max.x << "," << max.y << "," << max.z << std::endl;
    
    auto middlePoint = middlePointInAABB(min, max);
    
    std::cout << middlePoint.x << "," << middlePoint.y << "," << middlePoint.z << std::endl;
    
    auto length = longerAxisLength(min, max);
    
    auto centerSceneMatrix = glm::translate(glm::mat4(1.f), middlePoint);
    auto normalizeSizeMatrix = glm::scale(glm::mat4(1.f), glm::vec3(1.f / length));
    
    return normalizeSizeMatrix * centerSceneMatrix;
}

void Model::_processNode(aiNode *node, const aiScene *scene, glm::mat4 transformation) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        _meshes.push_back(_processMesh(mesh, scene, transformation));
    }
    
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        _processNode(node->mChildren[i], scene, transformation);
    }
}

Mesh Model::_processMesh(aiMesh *mesh, const aiScene *scene, glm::mat4 transformation) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec4 vector;
        
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vector.w = 1.f;
        
        auto scaled = transformation * vector;
        
        auto scaledxyz = glm::vec3 { scaled.x, scaled.y, scaled.z };
        scaledxyz /= scaled.w;
        
        vertex.Position = scaledxyz;
        
        vertices.push_back(vertex);
    }
    
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    
    return Mesh(vertices, indices, _device);
}
