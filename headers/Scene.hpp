#pragma once

#include <chrono>
#include <utility>

#include "Vulkan.hpp"
#include "Actor.hpp"
#include "Backdrop.hpp"
#include "Player.hpp"
#include "Light.hpp"
#include "Stage.hpp"
#include "Config.h"

class Scene
{
public:
    //config values
    tat::Vulkan *vulkan = nullptr;
    SceneConfig *config = nullptr;
    Player *player = nullptr;

    //generated values
    std::string name = "Unknown";
    Backdrop backdrop;
    Stage stage;
   
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    std::vector<Actor> actors;
    std::vector<Light> lights;

    ~Scene();

    void create();
    void cleanup();
    void recreate();

    void draw(VkCommandBuffer commandBuffer, uint32_t currentImage);

    void updateUniformBuffer(uint32_t currentImage);

    uint32_t numActors() { return static_cast<uint32_t>(actors.size()); };

    // num uniform buffers per model and stage (UBO and ULO)
    uint32_t numUniformBuffers() { return static_cast<uint32_t>((actors.size() + 1) * 2); };
    // same but for imagesamplers (diffuse, normal, roughness, ambientOcclusion) + sampler for stage
    uint32_t numImageSamplers() { return static_cast<uint32_t>((actors.size() + 1) * 4); };

private:
    float gamma = 4.5f;
    float exposure = 2.2f;

    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;

    void createLights();
    void createActors();
    void createBackdrop();
    void createStage();

    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createPipeline();
    void createPipelineLayout();
    void createUniformBuffers();
    void createDescriptorSets();
};