#pragma once

#include <imgui.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "Buffer.hpp"
#include "Vulkan.hpp"
#include "Timer.h"
#include "Image.hpp"

//sourced from https://github.com/SaschaWillems/Vulkan/blob/master/examples/imgui/main.cpp

// Options and values to display/toggle from the UI
struct UISettings
{
    bool displayModels = true;
    bool displayLogos = true;
    bool displayBackground = true;
    bool animateLight = false;
    float lightSpeed = 0.25f;
    std::array<float, 50> frameTimes{};
    float frameTimeMin = 9999.0f, frameTimeMax = 0.0f;
    float lightTimer = 0.0f;
};

// ----------------------------------------------------------------------------
// Overlay class
// ----------------------------------------------------------------------------
class Overlay
{
private:
    // Vulkan resources for rendering the UI
    VkSampler sampler{};
    Buffer vertexBuffer{};
    Buffer indexBuffer{};
    int32_t vertexCount = 0;
    int32_t indexCount = 0;

    Image fontImage{};

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;

    UISettings uiSettings;

    float lastFrametime = 0;

    void createDescriptorPool();
    void createDescriptorLayouts();
    void createDescriptorSets();
    void createPipelineLayout();
    void createPipeline();

public:
    tat::Vulkan *vulkan = nullptr;

    bool  update = false;
    // UI params are set via push constants
    struct PushConstBlock
    {
        glm::vec2 scale;
        glm::vec2 translate;
    } pushConstBlock;

    ~Overlay()
    {
        ImGui::DestroyContext();
        cleanup();
    }

    void cleanup()
    {
        vkDestroySampler(vulkan->device, sampler, nullptr);
        vkDestroyDescriptorSetLayout(vulkan->device, descriptorSetLayout, nullptr);
        vkDestroyPipeline(vulkan->device, pipeline, nullptr);
        vkDestroyPipelineLayout(vulkan->device, pipelineLayout, nullptr);
        vkDestroyDescriptorPool(vulkan->device, descriptorPool, nullptr);
    }

    void recreate()
    {
        initResources();
    }

    // Initialize styles, keys, etc.
    void init();

    // Initialize all Vulkan resources used by the ui
    void initResources();

    // Starts a new imGui frame and sets up windows and ui elements
    void newFrame(bool updateFrameGraph);

    // Update vertex and index buffer containing the imGui elements when required
    void updateBuffers();

    // Draw current imGui frame into a command buffer
    void draw(VkCommandBuffer commandBuffer, uint32_t currentImage);

    glm::vec3 *cameraPosition = nullptr;
    glm::vec3 *cameraRotation = nullptr;
};
