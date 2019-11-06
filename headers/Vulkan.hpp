#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#define GLM_FORCE_RADIANS
#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vk_mem_alloc.h>

#include <array>
#include <iostream>
#include <vector>

#include "helpers.hpp"

namespace tat
{

enum class Mode
{
    Game,
    Dbug,
    Nput,
    Free,
    Save,
    Load
};

struct UniformVertex
{
    glm::mat4 model{};
    glm::mat4 view{};
    glm::mat4 projection{};
    glm::mat4 lightMVP{};
    glm::mat4 normalMatrix{};
    glm::vec4 camPos{};
};

struct UniformBackdrop
{
    glm::mat4 inverseMVP{};
};

struct UniformLight
{
    glm::vec4 position{};
    glm::vec4 rotation{};
    glm::vec4 color{};
    float lumens = 0.0F;
    float steradians = 4.F * 3.1415926F;
};

struct UniformLights
{
    float radianceMipLevels = 0.F;
    float exposure = 2.2F;
    float gamma = 4.5F;
    float shadowSize = 1024.F;
    UniformLight light{};
    UniformLight flashLight{};
};

struct UniformShadow
{
    glm::mat4 model{};
    glm::mat4 view{};
    glm::mat4 projection{};
};

class Vulkan
{
  public:
    ~Vulkan()
    {
        device.destroyRenderPass(colorPass);
        device.destroyRenderPass(shadowPass);
        for (auto imageView : swapChainImageViews)
        {
            device.destroyImageView(imageView);
        }
        device.destroySwapchainKHR(swapChain);
        device.destroyCommandPool(commandPool);
        vmaDestroyAllocator(allocator);
        device.destroy(nullptr);

        instance.destroySurfaceKHR(surface);
        instance.destroy();
    };

    std::string name = "Unknown";

    vk::Instance instance;
    vk::SurfaceKHR surface;
    vk::PhysicalDevice physicalDevice;
    vk::PhysicalDeviceProperties properties;
    vk::Device device;
    VmaAllocator allocator;
    vk::CommandPool commandPool;
    vk::SwapchainKHR swapChain;
    std::vector<vk::Image> swapChainImages{};

    vk::RenderPass colorPass;
    vk::RenderPass shadowPass;

    vk::PresentModeKHR defaultPresentMode = vk::PresentModeKHR::eMailbox;
    vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;

    std::vector<vk::ImageView> swapChainImageViews;

    GLFWwindow *window = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t currentImage = 0;
    float zNear = 0.01F;
    float zFar = 512.F;
    float shadowSize = 1024.F;
    bool prepared = false;
    bool showOverlay = true;
    bool updateCommandBuffer = false;

    Mode mode = Mode::Dbug;

    std::string brdfPath;

    auto checkFormat(vk::Format format) -> bool
    {
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);
        return (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage) ==
               vk::FormatFeatureFlagBits::eSampledImage;
    };

    auto beginSingleTimeCommands() -> vk::CommandBuffer
    {
        vk::CommandBufferAllocateInfo allocInfo = {};
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        auto commandBuffer = device.allocateCommandBuffers(allocInfo);

        vk::CommandBufferBeginInfo beginInfo = {};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        commandBuffer[0].begin(beginInfo);

        return commandBuffer[0];
    };

    void endSingleTimeCommands(vk::CommandBuffer commandBuffer)
    {
        commandBuffer.end();

        vk::SubmitInfo submitInfo = {};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        graphicsQueue.submit(submitInfo, nullptr);
        graphicsQueue.waitIdle();
        device.freeCommandBuffers(commandPool, 1, &commandBuffer);
    };

    auto createShaderModule(const std::vector<char> &code) -> vk::ShaderModule
    {
        vk::ShaderModuleCreateInfo createInfo = {};
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        return device.createShaderModule(createInfo, nullptr);
    }

    auto findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
                             const vk::FormatFeatureFlags &features) -> vk::Format
    {
        for (vk::Format format : candidates)
        {
            auto props = physicalDevice.getFormatProperties(format);

            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features))
            {
                return format;
            }
            if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features))
            {
                return format;
            }
        }
        throw std::runtime_error("Failed to find supported format");
    }

    auto findDepthFormat() -> vk::Format
    {
        return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                   vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }
};

}; // end namespace tat