#pragma once

#include <map>

#include "Buffer.hpp"
#include "Collection.hpp"
#include "Vertex.hpp"


namespace tat
{

class Mesh : public Entry
{
  public:
    void load() override;
    virtual ~Mesh() = default;
    std::string path = "";
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

  private:
    void import();
};

}; // namespace tat