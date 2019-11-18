#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <optional>

#include <vk_mem_alloc.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "engine/Framebuffer.hpp"
#include "engine/RenderPass.hpp"
#include "engine/Allocator.hpp"

#include "Image.hpp"
#include "Debug.hpp"

namespace tat
{

const int MAX_FRAMES_IN_FLIGHT = 2;

struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    auto isComplete() -> bool
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Engine
{
  public:
    void create();
    void prepare();
    void destroy();
    void drawFrame(float deltaTime);

    bool showOverlay = false;
    bool updateCommandBuffer = false;

    vk::Instance instance;
    vk::SurfaceKHR surface;
    vk::Device device;

    Allocator allocator {};

    vk::PipelineCache pipelineCache;
    RenderPass colorPass;
    RenderPass shadowPass;

    std::vector<vk::Image> swapChainImages{};
    vk::PresentModeKHR defaultPresentMode = vk::PresentModeKHR::eMailbox;
    vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;
    vk::Format swapChainImageFormat = vk::Format::eR8G8B8A8Unorm;

    auto beginSingleTimeCommands() -> vk::CommandBuffer;
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

    auto createShaderModule(const std::string &filename) -> vk::ShaderModule;
    auto findDepthFormat() -> vk::Format;

  private:
    std::vector<Framebuffer> shadowFramebuffers{};
    Image shadowDepth;
    std::vector<Framebuffer> colorFramebuffers{};
    Image colorAttachment;
    Image depthAttachment;
    
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    std::vector<vk::CommandBuffer> commandBuffers{};
    std::vector<vk::ImageView> swapChainImageViews;
    std::vector<vk::Semaphore> presentFinishedSemaphores{};
    std::vector<vk::Semaphore> renderFinishedSemaphores{};
    std::vector<vk::Fence> waitFences{};

    vk::PhysicalDevice physicalDevice;
    vk::PhysicalDeviceProperties properties;

    Debug debug;

    vk::CommandPool commandPool;
    vk::SwapchainKHR swapChain;

    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

    vk::Extent2D swapChainExtent;

    int32_t currentImage = 0;
    bool prepared = false;

    void createCommandBuffers();

    void renderShadows(vk::CommandBuffer commandBuffer, int32_t currentImage);
    void renderColors(vk::CommandBuffer commandBuffer, int32_t currentImage);

    void updateWindow();
    void resizeWindow();

    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createColorFramebuffers();
    void createShadowFramebuffers();
    void createCommandPool();
    void createPipelineCache();
    void createSyncObjects();

    auto getRequiredExtensions() -> std::vector<const char *>;
    auto getMaxUsableSampleCount() -> vk::SampleCountFlagBits;
    static auto chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
        -> vk::SurfaceFormatKHR;
    auto chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) -> vk::PresentModeKHR;
    static auto chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, uint32_t windowWidth,
                                 uint32_t windowHeight) -> vk::Extent2D;

    auto isDeviceSuitable(vk::PhysicalDevice const &device) -> bool;
    auto findQueueFamiles(vk::PhysicalDevice const &device) -> QueueFamilyIndices;
    auto querySwapChainSupport(vk::PhysicalDevice const &device) -> SwapChainSupportDetails;
    auto checkDeviceExtensionsSupport(vk::PhysicalDevice const &device) -> bool;
};

} // namespace tat