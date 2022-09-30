#include "assimp/Importer.hpp"      // C++ importer interface
#include "assimp/scene.h"           // Output data structure
#include "assimp/postprocess.h"     // Post processing flags

class Object {
public:
    Object(const std::string& objectPath);

private:
    const aiScene* _scene;
};
