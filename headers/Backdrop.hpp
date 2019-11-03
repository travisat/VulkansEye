#pragma once

#include "Buffer.hpp"
#include "Config.h"
#include "Image.hpp"
#include "Light.hpp"
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
    BackdropConfig *config = nullptr;
    std::string name;

    bool loaded = false;

    Image *shadowMap;

    Image colorMap{};
    Image radianceMap{};
    Image irradianceMap{};

    Light light{};

    ~Backdrop();

    void create();
    void cleanup();
    void recreate();

    void draw(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage);

  private:
    UniformBuffer uBuffer;

    std::vector<vk::DescriptorSet> descriptorSets;
    std::vector<Buffer> uniformBuffers;
    vk::DescriptorPool descriptorPool;

    Pipeline pipeline;
    vk::DescriptorSetLayout descriptorSetLayout{};

    void loadCubeMap(Image &cubeMap, const std::string &path);
    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorSets();
    void createPipeline();
};

} // namespace tat