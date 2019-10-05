#pragma once

#include <optional>
#include <set>
#include <gsl/span>

#include "Vulkan.hpp"
#include "Scene.hpp"
#include "Overlay.hpp"

const int MAX_FRAMES_IN_FLIGHT = 2;

static bool framebufferResized = false;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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

class VkEngine
{
public:
    tat::Vulkan *vulkan = nullptr;
    Config *config = nullptr;
    Scene scene;
    Overlay overlay;

    ~VkEngine();
    void init();
    void drawFrame();

private:
    Image colorImage{};
    Image depthImage{};
    std::vector<VkFramebuffer> swapChainFramebuffers{};

    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    std::vector<VkCommandBuffer> commandBuffers{};

    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    std::vector<VkFence> inFlightFences{};
    size_t currentFrame = 0;

    void recordCommandBuffers();

    void createInstance();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createAllocator();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createColorResources();
    void createDepthResources();
    void createDescriptorSetLayouts();
    void createCommandBuffers();
    void createSyncObjects();

    std::vector<const char *> getRequiredExtensions();
    VkSampleCountFlagBits getMaxUsableSampleCount();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, uint32_t windowWidth, uint32_t windowHeight);
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat VkEngine::findDepthFormat();

    bool isDeviceSuitable(VkPhysicalDevice const &device);
    QueueFamilyIndices findQueueFamiles(VkPhysicalDevice const &device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice const &device);
    bool checkDeviceExtensionsSupport(VkPhysicalDevice const &device);

    void recreate(); //for screen size changes
};