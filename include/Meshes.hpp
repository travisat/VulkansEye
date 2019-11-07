#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cstdint>
#include <memory>

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
    std::shared_ptr<Vulkan> vulkan;
    void loadConfig(const MeshesConfig &config);
    auto getIndex(const std::string &name) -> int32_t;
    inline auto getMesh(int32_t index) -> Mesh *
    {
        if (index < collection.size() && index > 0)
        {
            return &collection[index];
        }
        //return default mesh if index out of range
        return &collection[0];
    };

  private:
    // vector of empty Meshes until mesh has been loaded
    std::vector<Mesh> collection{};
    // string index = mesh index
    std::map<std::string, int32_t> names{};

    void loadMesh(int32_t index);
    static void importMesh(Mesh *mesh);
};

}; // namespace tat