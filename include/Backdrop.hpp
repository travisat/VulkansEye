#pragma once

#include <memory>
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"
#include "Camera.hpp"
#include "Collection.hpp"
#include "Image.hpp"
#include "engine/Pipeline.hpp"

namespace tat
{

struct UniformBack
{
    glm::mat4 inverseMVP;
};

class Backdrop : public Entry
{
  public:
    glm::vec3 light{};

    Backdrop() = default;
    virtual ~Backdrop();

    std::shared_ptr<Image> colorMap;
    std::shared_ptr<Image> radianceMap;
    std::shared_ptr<Image> irradianceMap;

    void load() override;

    void recreate();

    void draw(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage);

  private:
    Pipeline pipeline;
    
    vk::DescriptorPool descriptorPool;
    vk::DescriptorSetLayout descriptorSetLayout{};
    std::vector<vk::DescriptorSet> descriptorSets;

    UniformBack backBuffer{};
    std::vector<Buffer> backBuffers;

    auto loadCubeMap(const std::string &file) -> std::shared_ptr<Image>;
    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorSets();
    void createPipeline();
};

} // namespace tat