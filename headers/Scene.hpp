#pragma once

#include <chrono>
#include <cstdint>
#include <utility>
#include <array>

#include "Actor.hpp"
#include "Backdrop.hpp"
#include "Config.h"
#include "Materials.hpp"
#include "Pipeline.hpp"
#include "Player.hpp"
#include "PointLight.hpp"
#include "Stage.hpp"
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
    void drawColor(VkCommandBuffer commandBuffer, uint32_t currentImage);
    void drawShadow(VkCommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage);

    auto numTessBuffers() -> uint32_t
    {
        return static_cast<uint32_t>(actors.size() + stage.models.size() + pointLights.size());
    };
    auto numUniformLights() -> uint32_t
    {
        return static_cast<uint32_t>(actors.size() + stage.models.size() + pointLights.size()) * numLights;
    };
    auto numImageSamplers() -> uint32_t
    {
        return static_cast<uint32_t>((actors.size() + stage.models.size() + pointLights.size()) * 6);
    };
    auto numShadows() -> uint32_t
    {
      return static_cast<uint32_t>(actors.size() + stage.models.size());
    };

  private:
    UniformLight uLight = {};
    VkDescriptorPool colorPool;
    VkDescriptorSetLayout colorLayout;
    VkDescriptorPool shadowPool;
    VkDescriptorSetLayout shadowLayout;

    Materials materials {};

    Stage stage;
    Pipeline colorPipeline;
    std::vector<Actor> actors;
    std::vector<PointLight> pointLights;

    Pipeline shadowPipeline;

    void createShadow();
    void createLights();
    void createMaterials();
    void createActors();
    void createBackdrop();
    void createStage();

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