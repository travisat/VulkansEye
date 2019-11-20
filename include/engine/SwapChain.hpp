#pragma once

#include <optional>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{
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

class SwapChain
{
  public:
    void create();
    void destroy();
    void recreate()
    {
        destroy();
        create();
    };

    vk::SwapchainKHR swapChain = nullptr;
    std::vector<vk::Image> images{};
    std::vector<vk::ImageView> imageViews{};

    vk::Format format = vk::Format::eR8G8B8A8Unorm;
    vk::Extent2D extent;
    int32_t count = 0;

    static auto findQueueFamiles(vk::PhysicalDevice const &physicalDevice) -> QueueFamilyIndices;
    static auto querySwapChainSupport(vk::PhysicalDevice const &physicalDevice) -> SwapChainSupportDetails;

  private:
    static auto chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
        -> vk::SurfaceFormatKHR;
    static auto chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes)
        -> vk::PresentModeKHR;
    static auto chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, uint32_t windowWidth,
                                 uint32_t windowHeight) -> vk::Extent2D;
};
} // namespace tat