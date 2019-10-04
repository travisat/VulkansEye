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

    void init();
    void create();
    void cleanup();
    void recreate();

    void draw(VkCommandBuffer &commandBuffer, uint32_t currentImage);

    void updateUniformBuffer(uint32_t currentImage);

    uint32_t numModels() { return static_cast<uint32_t>(models.size()); };

    //TODO make these pull from actual numbers
    // num uniform buffers per model and skybox uniform buffers
    uint32_t numUniformBuffers() { return static_cast<uint32_t>(models.size() * 2 + 1); };
    // same but for imagesamplers
    uint32_t numImageSamplers() { return static_cast<uint32_t>(models.size() * 4 + 1); };


    std::string name = "Unknown";
    Skybox *skybox = nullptr;
    Buffer vertexBuffer{};
    Buffer indexBuffer{};
    VkPipelineLayout pipelineLayout{};
    VkPipeline pipeline = VK_NULL_HANDLE;
    Camera camera{};

    std::vector<std::unique_ptr<Model>> models;
    std::vector<std::unique_ptr<Light>> lights;

private:
    float gamma = 4.5f;
    float exposure = 2.2f;

    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createDescriptorSets();
    void createPipelines();
};