#pragma once

#include "Buffer.hpp"
#include "State.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"
#include "Camera.hpp"
#include <memory>

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

    std::shared_ptr<Image> colorMap;
    std::shared_ptr<Image> radianceMap;
    std::shared_ptr<Image> irradianceMap;

    void load();

    void cleanup();
    void recreate();

    void draw(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage);

  private:
    std::shared_ptr<spdlog::logger> debugLogger;

    UniformBack backBuffer{};

    std::vector<vk::DescriptorSet> descriptorSets;
    std::vector<Buffer> backBuffers;
    vk::DescriptorPool descriptorPool;

    Pipeline pipeline;
    vk::DescriptorSetLayout descriptorSetLayout{};

    auto loadCubeMap(const std::string &path) -> std::shared_ptr<Image>;
    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorSets();
    void createPipeline();
};

} // namespace tat