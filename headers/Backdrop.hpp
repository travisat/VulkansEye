#pragma once

#include "Image.hpp"
#include "Buffer.hpp"
#include "Player.hpp"
#include "Timer.h"

class Backdrop
{
public:
    ~Backdrop();

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

    Player *player = nullptr;
    std::string path;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

private:
    Image cubeMap{};
    VkSampler sampler = VK_NULL_HANDLE;
    UniformBuffer uBuffer;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
 
    void loadCubeMap();
    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorSets();
    void createPipeline();
};