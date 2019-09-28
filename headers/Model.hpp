#pragma once

#include "Buffer.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "State.hpp"
#include "Config.h"

class Model
{
public:
    Model(State *state, uint32_t id, glm::vec3 position, Mesh *mesh, Material *material); //for already loaded mesh/material
    Model(State *state, uint32_t id, ModelType type, glm::vec3 position, std::string path); // for types supported that provide mesh/material
    ~Model();

    State *state;

    uint32_t id;
    glm::vec3 position;
    ModelType type;

    Mesh *mesh;
    Material *material;

    std::vector<Buffer *> uniformBuffers {};
    std::vector<Buffer *> uniformLights{};
    std::vector<VkDescriptorSet> descriptorSets {};
    
};