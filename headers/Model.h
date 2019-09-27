#pragma once



#include "Buffer.h"
#include "Mesh.h"
#include "Material.h"
#include "State.h"


 enum ModelType { obj, gltf };

class Model
{
public:
    Model(State *state, Mesh *mesh, Material *material, glm::vec3 position);
    Model(State *state, std::string path, ModelType type, glm::vec3 position);
    ~Model();

    State *state;

    uint32_t id;
    Mesh *mesh;
    Material *material;

    std::vector<Buffer *> uniformBuffers;
    std::vector<VkDescriptorSet> descriptorSets;

    glm::vec3 position;
};