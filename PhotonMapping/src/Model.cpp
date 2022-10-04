#include "./Model.hpp"

#include <iostream>

Model::Model(const std::string& objectPath, RTCDevice device) : _device(device) {
  _loadModel(objectPath);
}

void Model::commit(RTCScene scene) const  {
  for (auto mesh : _meshes) {
    mesh.commit(scene);
  }
}

std::unordered_map<RTCGeometry, Material> Model::getMaterialsMap() const {
  return _materials;
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

    // process ASSIMP's root node recursively
  _processNode(scene->mRootNode, scene);
}

void Model::_processNode(aiNode *node, const aiScene *scene) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    auto processedMesh = _processMesh(mesh, scene);
    _meshes.push_back(processedMesh);
    _materials.insert(processedMesh.getMaterialPair());
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    _processNode(node->mChildren[i], scene);
  }
}

Mesh Model::_processMesh(aiMesh *mesh, const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  // std::vector<Texture> textures;

  // walk through each of the mesh's vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    glm::vec3 vector;

    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;
    vertex.Position = vector;

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
