#pragma once

#include <unordered_map>

#include <tiny_obj_loader.h>

#include "Vertex.h"
#include "Config.h"
#include "Timer.h"

class Mesh
{
public:

    ~Mesh();
    std::string name = "Uknown Mesh";
    void loadConfig(const MeshConfig &config);
    uint32_t getId(){ return id;};

    uint32_t vertexSize = 0;
    uint32_t indexSize = 0;

    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;

    std::vector<Vertex> vertices = {};
    std::vector<uint32_t> indices = {};

private:
    int32_t id = 0;
};