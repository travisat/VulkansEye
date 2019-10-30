#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <utility>


#include "Backdrop.hpp"
#include "Config.h"
#include "Materials.hpp"
#include "Meshes.hpp"
#include "Pipeline.hpp"
#include "Player.hpp"
#include "PointLight.hpp"
#include "Model.hpp"
#include "Vulkan.hpp"

namespace tat
{

class Scene
{
  public:
    Vulkan *vulkan = nullptr;
    Config *config = nullptr;
    Player *player = nullptr;
    std::string name = "Unknown";

    Image shadow;

    ~Scene();

    void create();
    void cleanup();
    void recreate();
    void drawColor(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void drawShadow(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage);

    auto numTessBuffers() -> uint32_t
    {
        return static_cast<uint32_t>(models.size() + pointLights.size());
    };
    auto numUniformLights() -> uint32_t
    {
        return static_cast<uint32_t>(models.size() + pointLights.size()) * numLights;
    };
    auto numImageSamplers() -> uint32_t
    {
        return static_cast<uint32_t>((models.size() + pointLights.size()) * 6);
    };
    auto numShadows() -> uint32_t
    {
        return static_cast<uint32_t>(models.size());
    };

  private:
    UniformLight uLight = {};
    VkDescriptorPool colorPool;
    VkDescriptorSetLayout colorLayout;
    VkDescriptorPool shadowPool;
    VkDescriptorSetLayout shadowLayout;

    Backdrop backdrop;
    Materials materials{};
    Meshes meshes{};

    Pipeline colorPipeline;
    std::vector<Model> models;
    std::vector<PointLight> pointLights;

    Pipeline shadowPipeline;

    void createShadow();
    void createLights();
    void createMaterials();
    void createMeshes();
    void createModels();
    void createBackdrop();

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