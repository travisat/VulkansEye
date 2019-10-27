#pragma once

#include "vulkan/vulkan_core.h"
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
    alignas(16) glm::mat4 projection{};
    alignas(16) glm::mat4 view{};
    alignas(16) glm::mat4 model{};
};

static const int numLights = 1;
struct uPointLight
{
    glm::vec3 position{};
    float buffer = 0.0F;
    glm::vec3 color{};
    float lumens = 0.0F;
};

struct UniformLight
{
    std::array<uPointLight, numLights> light{};
};

struct TessControl
{
    float tessLevel = 64.0F;
};

struct TessEval
{
    glm::mat4 model{};
    glm::mat4 viewProjection{};
    float tessStrength = 0.1F;
    float tessAlpha = 0.3F;
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
        vkDestroyRenderPass(device, colorPass, nullptr);
        vkDestroyRenderPass(device, shadowPass, nullptr);
        for (auto imageView : swapChainImageViews)
        {
            vkDestroyImageView(device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroyCommandPool(device, commandPool, nullptr);
        vmaDestroyAllocator(allocator);
        vkDestroyDevice(device, nullptr);

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    };

    std::string name = "Unknown";

    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties properties;
    VkDevice device;
    VmaAllocator allocator;
    VkCommandPool commandPool;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages{};

    VkRenderPass colorPass;
    VkRenderPass shadowPass;

    VkPresentModeKHR defaultPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkImageView> swapChainImageViews;

    GLFWwindow *window = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t currentImage = 0;
    float zNear = 0.01F;
    float zFar = 512.0F;
    bool prepared = false;
    bool showOverlay = true;
    bool updateCommandBuffer = false;

    auto checkFormat(VkFormat format) -> bool
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        return (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != VK_FALSE;
    };

    auto beginSingleTimeCommands() -> VkCommandBuffer
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;

        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    };

    auto endSingleTimeCommands(VkCommandBuffer commandBuffer) -> VkResult
    {
        VkResult result = vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        return result;
    };

    auto createShaderModule(const std::vector<char> &code) -> VkShaderModule
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }
        return shaderModule;
    }

    auto findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                             VkFormatFeatureFlags features) -> VkFormat
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }
        throw std::runtime_error("Failed to find supported format");
    }

    auto findDepthFormat() -> VkFormat
    {
        return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                   VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
};

}; // end namespace tat