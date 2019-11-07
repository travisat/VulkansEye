#pragma once

#include "Buffer.hpp"
#include "Config.hpp"
#include "Image.hpp"
#include "Light.hpp"
#include "Pipeline.hpp"
#include "Player.hpp"
#include "Timer.hpp"
#include <memory>

namespace tat
{

class Backdrop
{
  public:
    std::shared_ptr<Vulkan> vulkan;
    std::shared_ptr<Player> player;
    BackdropConfig config;
    std::string name;

    bool loaded = false;

    const Image *shadowMap;

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
    UniformBack backBuffer;

    std::vector<vk::DescriptorSet> descriptorSets;
    std::vector<Buffer> backBuffers;
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