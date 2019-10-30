#pragma once

#include <tiny_obj_loader.h>

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

    void loadConfigs(const std::vector<MeshConfig> &meshes);

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
    static void loadObj(const std::string &path, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
};

}; // namespace tat