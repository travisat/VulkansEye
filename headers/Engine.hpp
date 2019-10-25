#pragma once

#include <optional>
#include <set>

#include "Overlay.hpp"
#include "Scene.hpp"
#include "Framebuffer.hpp"
#include "RenderPass.hpp"
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
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
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
    Vulkan *vulkan = nullptr;
    Scene *scene;
    Overlay *overlay;

    ~Engine();
    void init();
    void drawFrame();

  private:
    std::vector<Framebuffer> shadowFbs{};
    Image shadowColor{};
    Image shadowDepth{};
    std::vector<Framebuffer> swapChainFbs{};
    Image colorAttachment{};
    Image depthAttachment{};

    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    const std::vector<const char *> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    std::vector<VkCommandBuffer> commandBuffers{};

    std::vector<VkSemaphore> presentFinishedSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    std::vector<VkFence> waitFences{};

    void createCommandBuffers();
    //void recordColorCommandBuffers();

    void renderShadows(VkCommandBuffer commandBuffer, int32_t currentImage);
    void renderColors(VkCommandBuffer commandBuffer, int32_t currentImage);

    void updateWindow();
    void resizeWindow();

    void createInstance();
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createAllocator();
    void createSwapChain();
    void createColorFramebuffers();
    void createShadowFramebuffers();
    void createCommandPool();
    void createSyncObjects();

    static auto getRequiredExtensions() -> std::vector<const char *>;
    auto getMaxUsableSampleCount() -> VkSampleCountFlagBits;
    static auto chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) -> VkSurfaceFormatKHR;
    auto chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) -> VkPresentModeKHR;
    static auto chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, uint32_t windowWidth,
                                uint32_t windowHeight) -> VkExtent2D;

    auto isDeviceSuitable(VkPhysicalDevice const &device) -> bool;
    auto findQueueFamiles(VkPhysicalDevice const &device) -> QueueFamilyIndices;
    auto querySwapChainSupport(VkPhysicalDevice const &device) -> SwapChainSupportDetails;
    auto checkDeviceExtensionsSupport(VkPhysicalDevice const &device) -> bool;
};

} // namespace tat