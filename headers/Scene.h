#pragma once

#include <chrono>

#include "Model.h"
#include "Skybox.h"
#include "Input.h"
#include "Camera.h"
#include "Light.h"



class Scene
{
public:
    Scene(State *state, Config &config);
    ~Scene();

    void create();
    void cleanup();
    void recreate();

    void updateUniformBuffer(uint32_t currentImage);

    uint32_t numModels() { return static_cast<uint32_t>(models.size()); };

    Skybox *skybox;
    Buffer *vertexBuffer;
    Buffer *indexBuffer;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    std::map<uint32_t, Model *> models;
    std::map<uint32_t, Mesh *> meshes;
    std::map<uint32_t, Material *> materials;

    struct shaderValues
    {
        glm::vec4 lightDir;
        glm::vec3 color;
    } shaderValues;

private:
    State *state;
    Config *config;

    Camera camera;
    Light light;

    VkDescriptorSetLayout descriptorSetLayout;

    void createUniformBuffers();
    void createDescriptorSetLayouts();
    void createDescriptorSets();
    void createPipelines();
};