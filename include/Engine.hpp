#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <optional>

#include <vk_mem_alloc.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "Framebuffer.hpp"
#include "Image.hpp"
#include "RenderPass.hpp"

namespace tat
{

const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

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
    Engine() = default;
    ~Engine();
    void init();
    void prepare();
    void drawFrame(float deltaTime);

    bool showOverlay = false;
    bool updateCommandBuffer = false;

    vk::UniqueInstance instance;
    vk::UniqueSurfaceKHR surface;
    vk::UniqueDevice device;
    
    VmaAllocator allocator{};

    vk::UniquePipelineCache pipelineCache;
    vk::UniqueRenderPass colorPass;
    vk::UniqueRenderPass shadowPass;

    std::vector<vk::Image> swapChainImages{};
    vk::PresentModeKHR defaultPresentMode = vk::PresentModeKHR::eMailbox;
    vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;
    vk::Format swapChainImageFormat = vk::Format::eR8G8B8A8Unorm;

    auto beginSingleTimeCommands() -> vk::CommandBuffer;
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

    auto createShaderModule(const std::string &filename) -> vk::UniqueShaderModule;
    auto findDepthFormat() -> vk::Format;

  private:
    std::vector<Framebuffer> shadowFbs{};
    std::unique_ptr<Image> shadowDepth;
    std::vector<Framebuffer> swapChainFbs{};
    std::unique_ptr<Image> colorAttachment;
    std::unique_ptr<Image> depthAttachment;
    const std::vector<const char *> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    std::vector<vk::UniqueCommandBuffer> commandBuffers{};
    std::vector<vk::UniqueImageView> swapChainImageViews;
    std::vector<vk::UniqueSemaphore> presentFinishedSemaphores{};
    std::vector<vk::UniqueSemaphore> renderFinishedSemaphores{};
    std::vector<vk::UniqueFence> waitFences{};

    vk::PhysicalDevice physicalDevice;
    vk::PhysicalDeviceProperties properties;

    vk::UniqueCommandPool commandPool;
    vk::UniqueSwapchainKHR swapChain;

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
    void createAllocator();
    void createSwapChain();
    void createColorFramebuffers();
    void createShadowFramebuffers();
    void createCommandPool();
    void createPipelineCache();
    void createSyncObjects();

    static auto getRequiredExtensions() -> std::vector<const char *>;
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