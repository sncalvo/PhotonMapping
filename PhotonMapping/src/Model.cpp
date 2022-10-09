#include "./Model.hpp"

#include <iostream>

Model::Model(const std::string& objectPath, Material material, RTCDevice device) : _device(device) {
  _loadModel(objectPath, material);
}

Model::Model(const RTCGeometryType geometryType, Material material, RTCDevice device, glm::vec4 transform) : _device(device) {
  _loadPrimitive(geometryType, transform, material);
}

Model::Model(Material material, RTCDevice device, glm::vec3 corner, glm::vec3 uvec, glm::vec3 vvec) : _device(device) {
  _loadQuad(material, corner, uvec, vvec);
}

void Model::commit(RTCScene scene) const  {
  for (auto mesh : _meshes) {
    mesh.commit(scene);
  }
}

std::unordered_map<unsigned int, Material> Model::getMaterialsMap() const {
  return _materials;
}

void Model::_loadPrimitive(const RTCGeometryType geometryType, glm::vec4 transform, Material material) {
  auto mesh = Mesh(geometryType, _device, transform, material);

  _meshes.push_back(mesh);
  _materials.insert(mesh.getMaterialPair());
}

void Model::_loadQuad(Material material, glm::vec3 corner, glm::vec3 uvec, glm::vec3 vvec) {
  auto mesh = Mesh(material, corner, uvec, vvec, _device);

  _meshes.push_back(mesh);
  _materials.insert(mesh.getMaterialPair());
}

void Model::_loadModel(std::string const &path, Material material) {
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
  _processNode(scene->mRootNode, scene, material);
}

void Model::_processNode(aiNode *node, const aiScene *scene, Material material) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    auto processedMesh = _processMesh(mesh, scene, material);
    _meshes.push_back(processedMesh);
    _materials.insert(processedMesh.getMaterialPair());
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    _processNode(node->mChildren[i], scene, material);
  }
}

Mesh Model::_processMesh(aiMesh *mesh, const aiScene *scene, Material material) {
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

  return Mesh(vertices, indices, material, _device);
}
