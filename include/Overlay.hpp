#pragma once

#include <imgui.h>
#include <string_view>

#include "Buffer.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"

// sourced from
// https://github.com/SaschaWillems/Vulkan/blob/master/examples/imgui/main.cpp

namespace tat
{

// Options and values to display/toggle from the UI
struct UISettings
{
    glm::vec3 position = glm::vec3(0.F);
    glm::vec3 rotation = glm::vec3(0.F);

    float velocity = 0.F;
    float fps = 0.F;
    int32_t modeNum = 0;
};

constexpr std::array<std::string_view, 3> mode = {"Normal", "Visual", "Input"};

// ----------------------------------------------------------------------------
// Overlay class
// ----------------------------------------------------------------------------
class Overlay
{
  public:
    bool update = false;
    // UI params are set via push constants
    struct PushConstBlock
    {
        glm::vec2 scale;
        glm::vec2 translate;
    } pushConstBlock{};

    Overlay();
    ~Overlay();

    void recreate();
    void cleanup();

    // Starts a new imGui frame and sets up windows and ui elements
    void newFrame();

    // Update vertex and index buffer containing the imGui elements when required
    void updateBuffers();

    // Draw current imGui frame into a command buffer
    void draw(vk::CommandBuffer commandBuffer, uint32_t currentImage);

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
    // Vulkan resources for rendering the UI
    Buffer vertexBuffer{};
    Buffer indexBuffer{};
    int32_t vertexCount = 0;
    int32_t indexCount = 0;

    std::unique_ptr<Image> fontImage;

    Pipeline pipeline;
    vk::DescriptorPool descriptorPool;
    vk::DescriptorSetLayout descriptorSetLayout;
    std::vector<vk::DescriptorSet> descriptorSets;

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