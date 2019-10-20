#pragma once

#include "Buffer.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"
#include "Player.hpp"
#include "Timer.h"

namespace tat
{

class Backdrop
{
  public:
    Vulkan *vulkan = nullptr;
    Player *player = nullptr;
    std::string path;

    ~Backdrop();

    void create();
    void cleanup();
    void recreate();

    void draw(VkCommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage);

  private:
    Image cubeMap{};
    VkSampler sampler = VK_NULL_HANDLE;
    UniformBuffer uBuffer;

    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<Buffer> uniformBuffers;
    VkDescriptorPool descriptorPool;

    Pipeline pipeline;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

    void loadCubeMap();
    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorSets();
    void createPipeline();
};

} // namespace tat