#include "engine/SwapChain.hpp"
#include "State.hpp"

#include <spdlog/spdlog.h>

namespace tat
{

void SwapChain::create()
{
    auto &state = State::instance();
    auto &engine = state.engine;
    auto &window = state.at("settings").at("window");
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(engine.physicalDevice.device);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    format = surfaceFormat.format;

    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    extent = chooseSwapExtent(swapChainSupport.capabilities, window.at(0), window.at(1));

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.surface = engine.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    QueueFamilyIndices indices = findQueueFamiles(engine.physicalDevice.device);
    std::array<uint32_t, 2> queueFamilyIndices = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = nullptr;

    swapChain = engine.device.createSwapchain(createInfo);
    images = engine.device.getSwapchainImages(swapChain);
    count = images.size();
    imageViews.resize(count);

    for (size_t i = 0; i < images.size(); i++)
    {
        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = images[i];
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        imageViews[i] = engine.device.createImageView(viewInfo);
    }
    spdlog::info("Created SwapChain");
}

void SwapChain::destroy()
{
    auto &device = State::instance().engine.device;
    for (auto imageView : imageViews)
    {
        device.destroy(imageView);
    }
    if (swapChain)
    {

        device.destroy(swapChain);
    }
}

auto SwapChain::findQueueFamiles(vk::PhysicalDevice const &physicalDevice) -> QueueFamilyIndices
{
    auto &surface = State::instance().engine.surface;

    QueueFamilyIndices indices;

    auto queueFamilies = physicalDevice.getQueueFamilyProperties();

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
        {
            indices.graphicsFamily = i;
        }

        auto presentSupport = physicalDevice.getSurfaceSupportKHR(i, surface);

        if (queueFamily.queueCount > 0 && (presentSupport != VK_FALSE))
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }
        i++;
    }
    return indices;
}

auto SwapChain::querySwapChainSupport(vk::PhysicalDevice const &physicalDevice) -> SwapChainSupportDetails
{
    auto &surface = State::instance().engine.surface;
    SwapChainSupportDetails details{};
    details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    details.formats = physicalDevice.getSurfaceFormatsKHR(surface);
    details.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
    return details;
}

auto SwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
    -> vk::SurfaceFormatKHR
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

auto SwapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes)
    -> vk::PresentModeKHR
{
    auto &engine = State::instance().engine;
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == engine.defaultPresentMode)
        {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

auto SwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, uint32_t windowWidth,
                                 uint32_t windowHeight) -> vk::Extent2D
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }

    VkExtent2D actualExtent = {windowWidth, windowHeight};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

} // namespace tat