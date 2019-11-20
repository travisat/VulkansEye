#pragma once

#include <vector>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{
class PhysicalDevice
{
  public:
    void pick(const vk::Instance &instance);

    vk::PhysicalDevice device = nullptr;
    vk::PhysicalDeviceProperties properties;
    vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;
    const std::vector<const char *> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    auto createDevice(const vk::DeviceCreateInfo& createInfo) -> vk::Device
    {
      return device.createDevice(createInfo);
    }

    auto getFormatProperties(const vk::Format &format) -> vk::FormatProperties
    {
      return device.getFormatProperties(format);
    };

  private:
    auto getMaxUsableSampleCount() -> vk::SampleCountFlagBits;
    auto isDeviceSuitable(vk::PhysicalDevice const &device) -> bool;
    auto checkDeviceExtensionsSupport(vk::PhysicalDevice const &device) -> bool;
};
} // namespace tat