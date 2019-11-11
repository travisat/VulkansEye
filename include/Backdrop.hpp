#pragma once

#include "Buffer.hpp"
#include "Config.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"
#include "Camera.hpp"

namespace tat
{

class Backdrop
{
  public:
    std::shared_ptr<Vulkan> vulkan;
    std::shared_ptr<Camera> camera;
    std::string name;

    glm::vec3 light{};

    bool loaded = false;

    Backdrop() = default;
    ~Backdrop();

    Image colorMap{};
    Image radianceMap{};
    Image irradianceMap{};

    void loadConfig(const BackdropConfig &config);

    void cleanup();
    void recreate();

    void draw(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage);

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
    BackdropConfig config;

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