#pragma once

#include <chrono>

#include "Helpers.h"
#include "Model.h"
#include "Skybox.h"
#include "Input.h"
#include "Camera.h"

class Scene
{
public:
    Scene(State *state, Config &config);
    ~Scene();

    void create();
    void cleanup();
    void recreate();

    void updateUniformBuffer(uint32_t currentImage);


    VkBuffer getVertexBuffer() { return vertexBuffer->buffer; };
    VkBuffer getIndexBuffer() { return indexBuffer->buffer; };

    Mesh *getMesh(uint32_t id) { return meshes[id]; };
    Material *getMaterial(uint32_t id) { return materials[id]; };
    Model *getModel(uint32_t id) { return models[id]; };

    uint32_t numModels() { return static_cast<uint32_t>(models.size()); };

    Skybox *skybox;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    std::map<uint32_t, Model *> models;

private:
    State *state;

    Camera camera;

    Buffer *vertexBuffer;
    Buffer *indexBuffer;

    VkDescriptorSetLayout descriptorSetLayout;

    std::map<uint32_t, Mesh *> meshes;
    std::map<uint32_t, Material *> materials;

    Config *config;
    
    void createUniformBuffers();
    void createDescriptorSetLayouts();
    void createDescriptorSets();
    void createPipelines();
};