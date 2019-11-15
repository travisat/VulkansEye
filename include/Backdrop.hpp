#pragma once

#include <memory>

#include "Buffer.hpp"
#include "Camera.hpp"
#include "Collection.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"

namespace tat
{

struct UniformBack
{
    glm::mat4 inverseMVP;
};

class Backdrop : public Entry
{
  public:
    virtual ~Backdrop();
    glm::vec3 light{};

    std::shared_ptr<Image> colorMap;
    std::shared_ptr<Image> radianceMap;
    std::shared_ptr<Image> irradianceMap;

    void load() override;

    void cleanup();
    void recreate();

    void draw(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage);

  private:
    UniformBack backBuffer{};

    std::vector<vk::DescriptorSet> descriptorSets;
    std::vector<Buffer> backBuffers;
    vk::DescriptorPool descriptorPool;

    Pipeline pipeline;
    vk::DescriptorSetLayout descriptorSetLayout{};

    auto loadCubeMap(const std::string &file) -> std::shared_ptr<Image>;
    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorSets();
    void createPipeline();
};

} // namespace tat