#pragma once

#include "engine/Buffer.hpp"
#include "engine/Vertex.hpp"

#include "Collection.hpp"


namespace tat
{

class Mesh : public Entry
{
  public:
    void load() override;
    virtual ~Mesh() = default;
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
    void import(const std::string &file);
};

}; // namespace tat