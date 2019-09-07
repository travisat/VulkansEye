#include <vk_mem_alloc.h>

#include "Vertex.h"

struct Model
{
    std::string modelPath;
    uint32_t id;   

    uint32_t vertexSize = 0;
    uint32_t vertexOffset = 0;
    uint32_t indexSize = 0;
    uint32_t indexOffset = 0;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};