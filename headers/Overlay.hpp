#pragma once

#include <imgui.h>

#include "Buffer.hpp"
#include "Image.hpp"
#include "Player.hpp"
#include "Pipeline.hpp"
#include "Timer.h"
#include "Vulkan.hpp"

// sourced from
// https://github.com/SaschaWillems/Vulkan/blob/master/examples/imgui/main.cpp

namespace tat
{

// Options and values to display/toggle from the UI
struct UISettings
{
    glm::vec3 position = glm::vec3(0.0F);
    glm::vec3 rotation = glm::vec3(0.0F);
    float velocity = 0.0F;
    float fps = 0.0F;
};

// ----------------------------------------------------------------------------
// Overlay class
// ----------------------------------------------------------------------------
class Overlay
{
  public:
    tat::Vulkan *vulkan = nullptr;
    Player *player = nullptr;

    bool update = false;
    // UI params are set via push constants
    struct PushConstBlock
    {
        glm::vec2 scale;
        glm::vec2 translate;
    } pushConstBlock;

    ~Overlay();

    void create();
    void recreate();
    void cleanup();

    // Starts a new imGui frame and sets up windows and ui elements
    void newFrame();

    // Update vertex and index buffer containing the imGui elements when required
    void updateBuffers();

    // Draw current imGui frame into a command buffer
    void draw(VkCommandBuffer commandBuffer, uint32_t currentImage);

  private:
    // Vulkan resources for rendering the UI
    VkSampler sampler{};
    Buffer vertexBuffer{};
    Buffer indexBuffer{};
    int32_t vertexCount = 0;
    int32_t indexCount = 0;

    Image fontImage{};

    Pipeline pipeline;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;

    UISettings uiSettings;

    float lastFrameTime = 0;
    float lastUpdateTime = 0;
    float updateFreqTime = 0.1F; // time between updates

    void createBuffers();
    void createFont();
    void createDescriptorPool();
    void createDescriptorLayouts();
    void createDescriptorSets();
    void createPipeline();
};

} // namespace tat