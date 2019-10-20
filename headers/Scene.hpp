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

    ~Scene();

    void create();
    void cleanup();
    void recreate();
    void draw(VkCommandBuffer commandBuffer, uint32_t currentImage);
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
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;

    Stage stage;
    Pipeline pipeline;
    std::vector<Actor> actors;
    std::vector<PointLight> pointLights;

    Image shadowmap;

    void createLights();
    void createActors();
    void createBackdrop();
    void createStage();

    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createPipelines();
    void createUniformBuffers();
    void createDescriptorSets();
};

} // namespace tat