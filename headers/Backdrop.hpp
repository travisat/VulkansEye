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

    void draw(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage);

    Image *shadowMap;

  private:
    Image cubeMap{};
    UniformBuffer uBuffer;

    std::vector<vk::DescriptorSet> descriptorSets;
    std::vector<Buffer> uniformBuffers;
    vk::DescriptorPool descriptorPool;

    Pipeline pipeline;
    vk::DescriptorSetLayout descriptorSetLayout {};

    void loadCubeMap();
    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorSets();
    void createPipeline();
};

} // namespace tat