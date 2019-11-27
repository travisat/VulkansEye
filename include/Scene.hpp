#pragma once

#include <memory>
#include <vector>
#include <string>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "engine/Image.hpp"
#include "engine/Pipeline.hpp"

#include "Backdrop.hpp"
#include "Model.hpp"

namespace tat
{
class Scene
{
  public:
    std::string name = "Unknown";

    Image shadow{};
    Image brdf{};
    Backdrop *backdrop = nullptr;

    float shadowSize = 1024.F;

    void destroy();
    void create();
    void cleanup();
    void recreate();
    void drawColor(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void drawShadow(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage, float deltaTime);

  private:
    Pipeline colorPipeline;
    Pipeline shadowPipeline;

    vk::DescriptorPool colorPool = nullptr;
    vk::DescriptorSetLayout colorLayout = nullptr;
    vk::DescriptorPool shadowPool = nullptr;
    vk::DescriptorSetLayout shadowLayout = nullptr;

    UniformVert vertBuffer{};
    UniformFrag fragBuffer{};
    UniformShad shadBuffer{};

    std::vector<Model *> models{};

    void createBrdf();
    void createShadow();

    void loadModels();
    void loadBackdrop();

    void createColorPool();
    void createColorLayouts();
    void createColorPipeline();
    void createColorSets();

    void createShadowPool();
    void createShadowLayouts();
    void createShadowPipeline();
    void createShadowSets();
};

} // namespace tat