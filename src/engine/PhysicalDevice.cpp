#include "engine/PhysicalDevice.hpp"
#include "State.hpp"

#include <cstdint>
#include <set>
#include <string>

#include <spdlog/spdlog.h>

namespace tat
{

void PhysicalDevice::pick(const vk::Instance &instance)
{
    auto physicalDevices = instance.enumeratePhysicalDevices();

    assert(!physicalDevices.empty());

    for (const auto &physicalDevice : physicalDevices)
    {
        if (isDeviceSuitable(physicalDevice))
        {
            properties = physicalDevice.getProperties();
            spdlog::info("Device: {}", properties.deviceName);
            spdlog::info("ID: {}", properties.deviceID);
            spdlog::info("Type: {}", properties.deviceType);
            spdlog::info("Driver: {}", properties.driverVersion);
            spdlog::info("API: {}", properties.apiVersion);
            device = physicalDevice;
            msaaSamples = getMaxUsableSampleCount();
            return;
        }
    }

    if (!device)
    {
        spdlog::error("Unable to find suitable physical device");
        throw std::runtime_error("Unable to find suitable physical device");
    }
}

auto PhysicalDevice::isDeviceSuitable(vk::PhysicalDevice const &physicalDevice) -> bool
{
    auto indicies = SwapChain::findQueueFamiles(physicalDevice);
    auto extensionsSupported = checkDeviceExtensionsSupport(physicalDevice);
    bool swapChainAdequate = false;

    if (extensionsSupported)
    {
        auto swapChainSupport = SwapChain::querySwapChainSupport(physicalDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    auto supportedFeatures = physicalDevice.getFeatures();

    return indicies.isComplete() && extensionsSupported && swapChainAdequate &&
           (supportedFeatures.samplerAnisotropy != VK_FALSE) && (supportedFeatures.tessellationShader != VK_FALSE) &&
           (supportedFeatures.geometryShader != VK_FALSE);
};

auto PhysicalDevice::checkDeviceExtensionsSupport(vk::PhysicalDevice const &device) -> bool
{
    auto deviceExtensions = device.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

    for (const auto &deviceExtension : deviceExtensions)
    {
        requiredExtensions.erase(deviceExtension.extensionName);
    }

    return requiredExtensions.empty();
}

auto PhysicalDevice::getMaxUsableSampleCount() -> vk::SampleCountFlagBits
{
    const auto &colorCounts = properties.limits.framebufferColorSampleCounts;
    const auto &depthCounts = properties.limits.framebufferDepthSampleCounts;

    // Scoped enums are stupid

    uint8_t colors = 0;
    if (colorCounts & vk::SampleCountFlagBits::e64)
    {
        colors = 64;
    }
    else if (colorCounts & vk::SampleCountFlagBits::e32)
    {
        colors = 32;
    }
    else if (colorCounts & vk::SampleCountFlagBits::e16)
    {
        colors = 16;
    }
    else if (colorCounts & vk::SampleCountFlagBits::e8)
    {
        colors = 8;
    }
    else if (colorCounts & vk::SampleCountFlagBits::e4)
    {
        colors = 4;
    }
    else if (colorCounts & vk::SampleCountFlagBits::e2)
    {
        colors = 2;
    }
    else
    {
        colors = 1;
    }

    uint8_t depth = 0;
    if (depthCounts & vk::SampleCountFlagBits::e64)
    {
        depth = 64;
    }
    else if (depthCounts & vk::SampleCountFlagBits::e32)
    {
        depth = 32;
    }
    else if (depthCounts & vk::SampleCountFlagBits::e16)
    {
        depth = 16;
    }
    else if (depthCounts & vk::SampleCountFlagBits::e8)
    {
        depth = 8;
    }
    else if (depthCounts & vk::SampleCountFlagBits::e4)
    {
        depth = 4;
    }
    else if (depthCounts & vk::SampleCountFlagBits::e2)
    {
        depth = 2;
    }
    else
    {
        depth = 1;
    }

    uint8_t lowest = std::min(colors, depth);

    switch (lowest)
    {
    case 64:
        return vk::SampleCountFlagBits::e64;
    case 32:
        return vk::SampleCountFlagBits::e32;
    case 16:
        return vk::SampleCountFlagBits::e16;
    case 8:
        return vk::SampleCountFlagBits::e8;
    case 4:
        return vk::SampleCountFlagBits::e4;
    case 2:
        return vk::SampleCountFlagBits::e2;
    default:
        return vk::SampleCountFlagBits::e1;
    }
}


} // namespace tat