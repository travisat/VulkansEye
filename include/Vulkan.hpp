#pragma once

#include <memory>
#include <stdexcept>
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define GLM_FORCE_RADIANS
#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vk_mem_alloc.h>

#include <array>
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>


#include "Window.hpp"
#include "Config.hpp"

namespace tat
{

enum class DisplayMode
{
    cursor = 0,
    nocursor = 1
};

enum class Mode
{
    Game,
    Dbug,
    Nput,
    Free,
    Save,
    Load
};

struct UniformVert
{
    glm::mat4 model{};
    glm::mat4 view{};
    glm::mat4 projection{};
    glm::mat4 lightMVP{};
    glm::mat4 normalMatrix{};
    glm::vec4 camPos{};
};

struct UniformBack
{
    glm::mat4 inverseMVP{};
};

struct UniformFrag
{
    glm::vec4 position{};
    float radianceMipLevels = 0.F;
    float exposure = 2.2F;
    float gamma = 4.5F;
    float shadowSize = 1024.F;
};

struct UniformShad
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

    std::string name;
    std::shared_ptr<Window> window;
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
    std::vector<vk::ImageView> swapChainImageViews;

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t currentImage = 0;
    float zNear = 0.01F;
    float zFar = 512.F;
    float FoV = 67.F;
    float mouseSensitivity = 33.4F;
    float shadowSize = 1024.F;
    bool prepared = false;
    bool showOverlay = true;
    bool updateCommandBuffer = false;
    Mode mode = Mode::Dbug;
    vk::PresentModeKHR defaultPresentMode = vk::PresentModeKHR::eMailbox;

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

    auto createShaderModule(const std::string &filename) -> vk::ShaderModule
    {
        if (!std::filesystem::exists(filename))
        {
            spdlog::get("debugLogger")->error("Shader {} does not exist", filename);
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