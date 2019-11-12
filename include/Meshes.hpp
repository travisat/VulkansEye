#pragma once

#include <map>

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
    Meshes(const std::shared_ptr<Vulkan> &vulkan, const std::string &configPath);
    ~Meshes() = default;

    std::shared_ptr<Vulkan> vulkan;

    auto getMesh(const std::string &name) -> std::shared_ptr<Mesh>;

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
    // vector of empty Meshes until mesh has been loaded
    std::vector<std::shared_ptr<Mesh>> collection;
    // string index = mesh index
    std::map<std::string, int32_t> names{};

    void loadMesh(int32_t index);
    static void importMesh(const std::shared_ptr<Mesh> &mesh);
};

}; // namespace tat