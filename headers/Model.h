#pragma once

#include "Vertex.h"
#include "Helpers.h"

class Model
{
public:
    Model(std::string path);
    
    uint32_t vertexSize;
    uint32_t indexSize;

    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    uint32_t id;
};