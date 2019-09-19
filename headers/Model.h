#pragma once

#include "Buffer.h"
#include "Mesh.h"
#include "Material.h"
#include "State.h"
#include "Helpers.h"

class Model
{
public:
    Model(State *state, Mesh *mesh, Material *material);
    ~Model();

    State *state;

    uint32_t id;
    Mesh *mesh;
    Material *material;

    std::vector<Buffer *> uniformBuffers;
    std::vector<VkDescriptorSet> descriptorSets;

    double xpos;
    double ypos;
    double zpos;
};