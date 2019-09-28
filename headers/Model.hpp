#pragma once

#include "Buffer.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "State.hpp"
#include "Config.h"

class Model
{
public:
    Model(State *state, uint32_t id, glm::vec3 position, glm::vec3 scale, Mesh *mesh, Material *material); //for already loaded mesh/material
    Model(State *state, uint32_t id, ModelType type, glm::vec3 position, glm::vec3 scale, std::string path); // for types supported that provide mesh/material
    ~Model();

    State *state;

    uint32_t id;
    glm::vec3 position;
    glm::vec3 scale;
    ModelType type;

    Mesh *mesh;
    Material *material;

    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;
    uint32_t vertexSize = 0;
    uint32_t indexSize = 0;

    std::vector<Buffer *> uniformBuffers {};
    std::vector<Buffer *> uniformLights{};
    std::vector<VkDescriptorSet> descriptorSets {};
    
};