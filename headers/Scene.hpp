#pragma once

#include <chrono>
#include <utility>

#include "Vulkan.hpp"
#include "Model.hpp"
#include "Skybox.hpp"
#include "Input.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "Config.h"

class Scene
{
public:
    tat::Vulkan *vulkan = nullptr;
    Config *config = nullptr;

    ~Scene();

    void loadConfig(Config &config);

    void create();
    void cleanup();
    void recreate();

    void draw(VkCommandBuffer commandBuffer, uint32_t currentImage);

    void updateUniformBuffer(uint32_t currentImage);

    uint32_t numModels() { return static_cast<uint32_t>(models.size()); };

    // num uniform buffers per model (UBO and ULO)
    uint32_t numUniformBuffers() { return static_cast<uint32_t>(models.size() * 2); };
    // same but for imagesamplers (diffuse, normal, roughness, ambientOcclusion)
    uint32_t numImageSamplers() { return static_cast<uint32_t>(models.size() * 4); };

    std::string name = "Unknown";
    Skybox skybox;
    Buffer vertexBuffer;
    Buffer indexBuffer;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    Camera camera;

    //use unique ptrs so destroyed on destruction of scene
    std::vector<Model *> models;
    std::vector<Light *> lights;

private:
    float gamma = 4.5f;
    float exposure = 2.2f;

    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;

    void loadLights();
    void loadModels();

    void createBuffers(); //put models meshes into vertex/index buffers

    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createPipeline();
    void createUniformBuffers();
    void createDescriptorSets();
};