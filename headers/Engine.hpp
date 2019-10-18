#pragma once

#include <optional>
#include <set>

#include "Overlay.hpp"
#include "Scene.hpp"
#include "Framebuffer.hpp"
#include "Vulkan.hpp"

namespace tat
{

const int MAX_FRAMES_IN_FLIGHT = 2;

static bool framebufferResized = false;

const std::vector<const char *> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Engine
{
  public:
    Vulkan *vulkan = nullptr;
    Scene *scene;
    Overlay *overlay;

    ~Engine();
    void init();
    void drawFrame();

  private:
    std::vector<Framebuffer> swapChainFbs{};
    Image colorAttachment{};
    Image depthAttachment{};
    VkSampler colorSampler;

    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    std::vector<VkCommandBuffer> commandBuffers{};
    VkCommandBuffer offscreenCommandBuffer = VK_NULL_HANDLE;

    std::vector<VkSemaphore> presentFinishedSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    std::vector<VkFence> waitFences{};

    void createCommandBuffers();
    void recordCommandBuffers();

    void createDeferredCommandBuffer();

    void updateWindow();
    void resizeWindow();

    void createInstance();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createAllocator();
    void createSwapChain();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createSyncObjects();

    std::vector<const char *> getRequiredExtensions();
    VkSampleCountFlagBits getMaxUsableSampleCount();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, uint32_t windowWidth,
                                uint32_t windowHeight);

    bool isDeviceSuitable(VkPhysicalDevice const &device);
    QueueFamilyIndices findQueueFamiles(VkPhysicalDevice const &device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice const &device);
    bool checkDeviceExtensionsSupport(VkPhysicalDevice const &device);
};

} // namespace tat