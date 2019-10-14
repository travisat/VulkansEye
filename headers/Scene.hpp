#pragma once

#include <chrono>
#include <utility>
#include <glm/gtx/matrix_transform_2d.hpp>

#include "Vulkan.hpp"
#include "Actor.hpp"
#include "Backdrop.hpp"
#include "Player.hpp"
#include "PointLight.hpp"
#include "Stage.hpp"
#include "Config.h"

namespace tat
{

class Scene
{
public:
    //config values
    tat::Vulkan *vulkan = nullptr;
    SceneConfig *config = nullptr;
    Player *player = nullptr;

    //generated values
    std::string name = "Unknown";
    Stage stage;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    std::vector<Actor> actors;
    std::vector<PointLight> pointLights;

    ~Scene();

    void create();
    void cleanup();
    void recreate();

    void draw(VkCommandBuffer commandBuffer, uint32_t currentImage);

    void updateUniformBuffer(uint32_t currentImage);

    uint32_t numActors() { return static_cast<uint32_t>(actors.size()); };

    // num uniform buffers per model and stage (UBO and ULO)
    uint32_t numUniformBuffers() { return static_cast<uint32_t>(actors.size() + 1); };
    uint32_t numTessBuffers() { return static_cast<uint32_t>((actors.size() + 1) * 2); };
    uint32_t numUniformLights() { return static_cast<uint32_t>(actors.size() + 1); };
    // same but for imagesamplers (diffuse, normal, roughness, metllalic, ambientOcclusion) + sampler for stage
    uint32_t numImageSamplers() { return static_cast<uint32_t>((actors.size() + 1) * 6); };

private:
    UniformBuffer uBuffer;
    UniformLight uLight;
    TessControl uTessControl;
    TessEval uTessEval;

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

} //namespace tat