#pragma once

#include <memory>
#include <optional>
#include <set>

#include "Framebuffer.hpp"
#include "Overlay.hpp"
#include "RenderPass.hpp"
#include "Scene.hpp"
#include "Vulkan.hpp"

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
    std::shared_ptr<Vulkan> vulkan;
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Overlay> overlay;

    ~Engine();
    void init();
    void drawFrame();

  private:
    vk::DispatchLoaderDynamic dldy;

    std::vector<Framebuffer> shadowFbs{};
    Image shadowDepth{};
    std::vector<Framebuffer> swapChainFbs{};
    Image colorAttachment{};
    Image depthAttachment{};

    const std::vector<const char *> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    std::vector<vk::CommandBuffer> commandBuffers{};

    std::vector<vk::Semaphore> presentFinishedSemaphores{};
    std::vector<vk::Semaphore> renderFinishedSemaphores{};
    std::vector<vk::Fence> waitFences{};

    void createCommandBuffers();
    // void recordColorCommandBuffers();

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