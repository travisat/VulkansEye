#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>


#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define GLM_FORCE_RADIANS
#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>


namespace tat
{

class Vulkan
{
  public:
    ~Vulkan()
    {
        if (colorPass)
        {
            device.destroyRenderPass(colorPass);
        }
        if (shadowPass)
        {
            device.destroyRenderPass(shadowPass);
        }

        if (swapChain)
        {
            device.destroySwapchainKHR(swapChain);
        }

        if (pipelineCache)
        {
            device.destroyPipelineCache(pipelineCache);
        }
        
        if (commandPool)
        {
            device.destroyCommandPool(commandPool);
        }
        if (allocator != nullptr)
        {
            vmaDestroyAllocator(allocator);
        }
        if (device)
        {
            device.destroy(nullptr);
        }

        if (surface)
        {
            instance.destroySurfaceKHR(surface);
        }
        if (instance)
        {
            instance.destroy();
        }
    };

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
    vk::SampleCountFlagBits msaaSamples;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    vk::PipelineCache pipelineCache;

    vk::PresentModeKHR defaultPresentMode = vk::PresentModeKHR::eMailbox;

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

    auto createShaderModule(const std::string &filename) -> vk::ShaderModule
    {
        if (!std::filesystem::exists(filename))
        {
            spdlog::error("Shader {} does not exist", filename);
            throw std::runtime_error("Shader does not exist");
            return nullptr;
        }

        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        vk::ShaderModuleCreateInfo createInfo = {};
        createInfo.codeSize = buffer.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(buffer.data());

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