#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cstdint>

#include "Buffer.hpp"
#include "Config.hpp"
#include "Vertex.hpp"

namespace tat
{

struct Mesh
{
    std::string name = "";
    std::string path = "";
    glm::vec3 center{};
    glm::vec3 size{};

    struct
    {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
    } data;

    struct
    {
        Buffer vertex{};
        Buffer index{};
    } buffers;

    bool loaded = false;
};

class Meshes
{
  public:
    Vulkan *vulkan;

    void loadConfig(const MeshesConfig &config);

    auto getMesh(const std::string &name) -> Mesh *;

  private:
    // vector of empty Meshes until mesh has been loaded
    std::vector<Mesh> collection{};
    // string index = mesh index
    std::map<std::string, int32_t> names{};

    auto getIndex(const std::string &name) -> int32_t;
    void loadMesh(int32_t index);
    static void importMesh(Mesh *mesh);
};

}; // namespace tat