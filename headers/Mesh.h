#pragma once

#include "Vertex.h"
#include "Helpers.h"
#include "Config.h"

class Mesh
{
public:
    //reserve id 0 for Skybox
    Mesh(MeshConfig const &config);
    
    uint32_t getId(){ return id;};

    uint32_t vertexSize;
    uint32_t indexSize;

    uint32_t vertexOffset;
    uint32_t indexOffset;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

private:
    uint32_t id;
};