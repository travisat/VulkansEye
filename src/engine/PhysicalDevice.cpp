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
    for (const auto &physicalDevice : instance.enumeratePhysicalDevices())
    {
        if (isDeviceSuitable(physicalDevice))
        {
            properties = physicalDevice.getProperties();

            if constexpr (Debug::enable)
            {
                spdlog::info("Device: {}", properties.deviceName);
                spdlog::info("ID: {}", properties.deviceID);
                spdlog::info("Type: {}", properties.deviceType);
                spdlog::info("Driver: {}", properties.driverVersion);
                spdlog::info("API: {}", properties.apiVersion);
            }

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
    auto indiciesComplete = SwapChain::findQueueFamiles(physicalDevice).isComplete();

    auto swapChainAdequate = false;
    if (checkDeviceExtensionsSupport(physicalDevice))
    {
        auto swapChainSupport = SwapChain::querySwapChainSupport(physicalDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    auto supportedFeatures = physicalDevice.getFeatures();
    auto featuresSupported = true;
    featuresSupported = featuresSupported && (supportedFeatures.samplerAnisotropy != VK_FALSE);
    featuresSupported = featuresSupported && (supportedFeatures.geometryShader != VK_FALSE);

    return indiciesComplete && swapChainAdequate && featuresSupported;
};

auto PhysicalDevice::checkDeviceExtensionsSupport(vk::PhysicalDevice const &device) -> bool
{
    std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());
    for (const auto &deviceExtension : device.enumerateDeviceExtensionProperties())
    {
        requiredExtensions.erase(deviceExtension.extensionName);
    }

    return requiredExtensions.empty();
}

auto getSamples(const vk::SampleCountFlags& sampleFlag) -> uint8_t
{
    if (sampleFlag & vk::SampleCountFlagBits::e64)
    {
        return 64;
    }
    if (sampleFlag & vk::SampleCountFlagBits::e32)
    {
        return 32;
    }
    if (sampleFlag & vk::SampleCountFlagBits::e16)
    {
        return 16;
    }
    if (sampleFlag & vk::SampleCountFlagBits::e8)
    {
        return 8;
    }
    if (sampleFlag & vk::SampleCountFlagBits::e4)
    {
        return 4;
    }
    if (sampleFlag & vk::SampleCountFlagBits::e2)
    {
        return 2;
    }
    return 1;
}

auto PhysicalDevice::getMaxUsableSampleCount() -> vk::SampleCountFlagBits
{
    auto colors = getSamples(properties.limits.framebufferColorSampleCounts);
    auto depths = getSamples(properties.limits.framebufferDepthSampleCounts);

    switch (std::min(colors, depths))
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