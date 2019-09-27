#pragma once

#include <chrono>

#include "Model.h"
#include "Skybox.h"
#include "Input.h"
#include "Camera.h"
#include "Light.h"
#include "Config.h"

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
    Camera camera;

    std::map<uint32_t, Model *> models;
    std::map<uint32_t, Mesh *> meshes;
    std::map<uint32_t, Material *> materials;
    std::map<uint32_t, Light *> lights;

    struct uniformLightObject
    {
        glm::vec3 color;
        glm::vec4 direction;
    };

    std::vector<Buffer *> uniformLights{};

private:
    State *state;
    Config *config;

    VkDescriptorSetLayout descriptorSetLayout;

    void createUniformBuffers();
    void createDescriptorSetLayouts();
    void createDescriptorSets();
    void createPipelines();
};