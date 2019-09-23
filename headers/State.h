#pragma once

#include "Helpers.h"

class State
{
public:
    State();
    ~State();
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    VkShaderModule createShaderModule(const std::vector<char> &code);

    VkDevice device;

    VkRenderPass renderPass;

    GLFWwindow *window;
    uint32_t windowWidth;
    uint32_t windowHeight;

    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    VmaAllocator allocator;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkCommandPool commandPool;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkImageView> swapChainImageViews;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    VkDescriptorPool descriptorPool;

};