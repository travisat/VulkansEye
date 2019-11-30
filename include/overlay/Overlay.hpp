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
#include "overlay/Info.hpp"

// sourced from
// https://github.com/SaschaWillems/Vulkan/blob/master/examples/imgui/main.cpp

namespace tat
{

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

    struct
    {
        bool showEditor = true;
        bool showInfo = false;
        bool showExit = false;
    } settings;

  private:
    // Vulkan resources for rendering the UI
    Buffer vertexBuffer{};
    Buffer indexBuffer{};

    Image fontImage{};

    Editor editor{};
    Info info{};

    ImGuiIO *io = nullptr;
    Window *window = nullptr;

    Pipeline pipeline{};
    vk::DescriptorPool descriptorPool = nullptr;
    vk::DescriptorSetLayout descriptorSetLayout = nullptr;
    std::vector<vk::DescriptorSet> descriptorSets{};

    std::array<GLFWcursor *, ImGuiMouseCursor_COUNT> g_MouseCursors{};

    void createBuffers();
    void createFont();
    void createDescriptorPool();
    void createDescriptorLayouts();
    void createDescriptorSets();
    void createPipeline();
};

} // namespace tat