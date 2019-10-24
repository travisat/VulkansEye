#pragma once

#include <chrono>
#include <utility>

#include "Actor.hpp"
#include "Backdrop.hpp"
#include "Config.h"
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
    SceneConfig *config = nullptr;
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

    uint32_t numTessBuffers()
    {
        return static_cast<uint32_t>(actors.size() + stage.models.size() + pointLights.size());
    };
    uint32_t numUniformLights()
    {
        return static_cast<uint32_t>(actors.size() + stage.models.size() + pointLights.size()) * numLights;
    };
    uint32_t numImageSamplers()
    {
        return static_cast<uint32_t>((actors.size() + stage.models.size() + pointLights.size()) * 6);
    };

  private:
    UniformLight uLight = {};
    VkDescriptorPool colorPool;
    VkDescriptorSetLayout colorLayout;
    VkDescriptorPool shadowPool;
    VkDescriptorSetLayout shadowLayout;

    Stage stage;
    Pipeline colorPipeline;
    std::vector<Actor> actors;
    std::vector<PointLight> pointLights;

    Pipeline shadowPipeline;
    
    VkSampler shadowSampler;

    void createShadow();
    void createLights();
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