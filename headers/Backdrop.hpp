#pragma once

#include "Pipeline.hpp"
#include "Image.hpp"
#include "Buffer.hpp"
#include "Player.hpp"
#include "Timer.h"

namespace tat
{

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

private:
    Image cubeMap{};
    VkSampler sampler = VK_NULL_HANDLE;
    UniformBuffer uBuffer;

    Pipeline pipeline;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

    void loadCubeMap();
    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorSets();
    void createPipeline();
};

} //namespace tat