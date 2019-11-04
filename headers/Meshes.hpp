#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cstdint>

#include "Buffer.hpp"
#include "Config.h"
#include "Vertex.h"

namespace tat
{

struct Mesh
{
    std::string name = "";
    uint32_t vertexSize = 0;
    uint32_t indexSize = 0;
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};
    Buffer vertexBuffer{};
    Buffer indexBuffer{};
    bool loaded = false;
};

class Meshes
{
  public:
    Vulkan *vulkan;

    void loadConfig(const MeshesConfig &config);

    auto getMesh(const std::string &name) -> Mesh *;

  private:
    // vector of configs, use loadConfigs to populate
    std::vector<MeshConfig> configs{};
    // vector of empty Meshes until mesh has been loaded
    std::vector<Mesh> collection{};
    // string index = mesh index
    std::map<std::string, int32_t> names{};

    auto getIndex(const std::string &name) -> int32_t;
    void loadMesh(int32_t index);
    static void importMesh(const std::string &path, Mesh *mesh);
};

}; // namespace tat