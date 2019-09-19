#pragma once


#include "Helpers.h"
#include "State.h"
#include "Input.h"
#include "Scene.h"
#include "Buffer.h"
#include "Image.h"
#include "Skybox.h"
#include "Config.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

static bool framebufferResized = false;


const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

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

class VkBackend
{
public:
    VkBackend(GLFWwindow *glwindow, uint32_t width, uint32_t height, Config &config);
    ~VkBackend();
    void drawFrame();
    void initVulkan();

    State* state;
 

private:
    std::vector<VkFramebuffer> swapChainFramebuffers;

    Scene* scene;

    Image* colorImage;
    Image* depthImage;
  
    VkDebugUtilsMessengerEXT debugMessenger;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    size_t currentFrame = 0;

    void createInstance();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
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
    void loadScene();
    void createDescriptorSetLayouts();
    void createDescriptorPool();

    void createCommandBuffers();
    void createSyncObjects();


    std::vector<const char *> getRequiredExtensions();
    VkSampleCountFlagBits getMaxUsableSampleCount();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, uint32_t windowWidth, uint32_t windowHeight);

    bool isDeviceSuitable(VkPhysicalDevice const &device);
    QueueFamilyIndices findQueueFamiles(VkPhysicalDevice const &device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice const &device);
    bool checkDeviceExtensionsSupport(VkPhysicalDevice const &device);

    void recreateSwapChain();
    void cleanupSwapChain();
};