#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <utility>

#include "Backdrops.hpp"
#include "Config.hpp"
#include "Light.hpp"
#include "Materials.hpp"
#include "Meshes.hpp"
#include "Model.hpp"
#include "Pipeline.hpp"
#include "Player.hpp"
#include "Vulkan.hpp"

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
    Image brdf;

    ~Scene();

    void create();
    void cleanup();
    void recreate();
    void drawColor(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void drawShadow(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage);

  private:
    vk::DescriptorPool colorPool;
    vk::DescriptorSetLayout colorLayout;
    vk::DescriptorPool shadowPool;
    vk::DescriptorSetLayout shadowLayout;

    Backdrop* backdrop;

    UniformVertex vertexBuffer{};
    UniformLights lightsBuffer{};
    UniformShadow shadowBuffer{};

    Pipeline colorPipeline;
    std::vector<Model> models;
    std::vector<Light> lights;

    Pipeline shadowPipeline;

    void createBrdf();
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
};

} // namespace tat