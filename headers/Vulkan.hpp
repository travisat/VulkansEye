#pragma once

#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "helpers.h"
#include "vulkan/vulkan_core.h"
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

namespace tat
{

struct UniformBuffer
{
    alignas(16) glm::mat4 mvp{};
};

static const int numLights = 1;
struct uPointLight
{
    alignas(16) glm::vec3 position{};
    alignas(16) glm::vec3 color{};
    float lumens = 0.0F;
};

struct UniformLight
{
    alignas(16) glm::vec3 sunAngle = glm::vec3(-20.F, 20.F, -40.F);
    alignas(16) glm::mat4 sunMVP{};
    alignas(4) float radianceMipLevels = 0.F;
    alignas(4) float exposure = 2.2F;
    alignas(4) float gamma = 4.5F;
    alignas(4) float buffer = 0.F;
    std::array<uPointLight, numLights> light{};
};

struct UniformShadow
{
    glm::mat4 model{};
    std::array<glm::mat4, 6> view{};
    glm::mat4 projection{};
    glm::vec4 lightpos{};
};

class Vulkan
{
  public:
    ~Vulkan()
    {
        device.destroyRenderPass(colorPass);
        device.destroyRenderPass(shadowPass);
        device.destroyRenderPass(sunPass);
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
    vk::RenderPass sunPass;

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
    float zFar = 512.0F;
    bool prepared = false;
    bool showOverlay = true;
    bool updateCommandBuffer = false;

    auto checkFormat(vk::Format format) -> bool
    {
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);
        return (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage) == vk::FormatFeatureFlagBits::eSampledImage;
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
                             const vk::FormatFeatureFlags& features) -> vk::Format
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