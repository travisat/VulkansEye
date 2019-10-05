#pragma once

#include "Image.hpp"
#include "Buffer.hpp"
#include "Camera.hpp"
#include "Timer.h"

class Skybox
{
public:
    ~Skybox();

    void create();
    void cleanup();
    void recreate();

    void draw(VkCommandBuffer commandBuffer, uint32_t currentImage);

    void updateUniformBuffer(uint32_t currentImage);

    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<Buffer> uniformBuffers;
    VkDescriptorPool descriptorPool;

    tat::Vulkan *vulkan = nullptr;
    std::string name = "Uknown";

    Camera *camera = nullptr;
    std::string path;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

private:
    Image cubeMap{};
    VkSampler sampler = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
 

    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorSets();
    void createPipeline();
};