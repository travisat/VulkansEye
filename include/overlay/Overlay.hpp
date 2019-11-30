#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include <imgui.h>
#include <string_view>

#include "engine/Buffer.hpp"
#include "engine/Image.hpp"
#include "engine/Pipeline.hpp"
#include "engine/Window.hpp"
#include "overlay/Editor.hpp"

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

    bool showEditor = true;
    bool showInfo = false;
};

constexpr std::array<std::string_view, 3> mode = {"Normal", "Visual", "Input"};

// ----------------------------------------------------------------------------
// Overlay class
// ----------------------------------------------------------------------------
class Overlay
{
  public:
    // UI params are set via push constants
    struct PushConstBlock
    {
        glm::vec2 scale;
        glm::vec2 translate;
    } pushConstBlock{};

    void create();
    void destroy();
    void recreate();
    void cleanup();

    // Starts a new imGui frame and sets up windows and ui elements
    void update(float deltaTime);

    // Draw current imGui frame into a command buffer
    void draw(vk::CommandBuffer commandBuffer, uint32_t currentImage);

    UISettings uiSettings{};

  private:
    // Vulkan resources for rendering the UI
    Buffer vertexBuffer{};
    Buffer indexBuffer{};

    Image fontImage{};

    Editor editor{};

    ImGuiIO *io = nullptr;
    Window *window = nullptr;

    Pipeline pipeline{};
    vk::DescriptorPool descriptorPool = nullptr;
    vk::DescriptorSetLayout descriptorSetLayout = nullptr;
    std::vector<vk::DescriptorSet> descriptorSets{};

    float lastFrameTime = 0;
    float lastUpdateTime = 0;
    float updateFreqTime = 0.1F; // time between updates

    void showInfo();

    void createBuffers();
    void createFont();
    void createDescriptorPool();
    void createDescriptorLayouts();
    void createDescriptorSets();
    void createPipeline();
};

} // namespace tat