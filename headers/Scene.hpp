#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <utility>

#include "Backdrops.hpp"
#include "Config.h"
#include "Light.hpp"
#include "Materials.hpp"
#include "Meshes.hpp"
#include "Model.hpp"
#include "Pipeline.hpp"
#include "Player.hpp"
#include "Vulkan.hpp"
#include "glm/fwd.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"

namespace tat
{

class Scene
{
  public:
    Vulkan *vulkan = nullptr;
    Player *player = nullptr;
    Materials *materials = nullptr;
    Meshes *meshes = nullptr;
    Backdrops *backdrops = nullptr;

    std::string name = "Unknown";

    SceneConfig config{};
    Image shadow;
    Image sun;
    Image brdf;

    ~Scene();

    void create();
    void cleanup();
    void recreate();
    void drawColor(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void drawShadow(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void drawSun(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage);

  private:
    VkDescriptorPool colorPool;
    VkDescriptorSetLayout colorLayout;
    VkDescriptorPool shadowPool;
    VkDescriptorSetLayout shadowLayout;
    VkDescriptorPool sunPool;
    VkDescriptorSetLayout sunLayout;

    Backdrop* backdrop;

    UniformBuffer vertex{};
    UniformLight uLight{};
    UniformShadow shadowmvp{};
    UniformSun sunmvp{};

    Pipeline colorPipeline;
    std::vector<Model> models;
    std::vector<Light> lights;

    Pipeline shadowPipeline;
    Pipeline sunPipeline;

    void createBrdf();
    void createSun();
    void createShadow();
    void createModels();

    void loadBackdrop();

    void createColorPool();
    void createColorLayouts();
    void createColorPipeline();
    void createColorSets();

    void createShadowPool();
    void createShadowLayouts();
    void createShadowPipeline();
    void createShadowSets();

    void createSunPool();
    void createSunLayouts();
    void createSunPipeline();
    void createSunSets();
};

} // namespace tat