#include "State.hpp"

#include <cstdint>
#include <set>
#include <vector>

#include <spdlog/spdlog.h>

namespace tat
{

void Device::create()
{
    auto &engine = State::instance().engine;
    QueueFamilyIndices indices = SwapChain::findQueueFamiles(engine.physicalDevice.device);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0F;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;
    deviceFeatures.geometryShader = VK_TRUE;

    vk::DeviceCreateInfo createInfo{};
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = engine.physicalDevice.extensions.size();
    createInfo.ppEnabledExtensionNames = engine.physicalDevice.extensions.data();
    createInfo.enabledLayerCount = 0;

    device = engine.physicalDevice.createDevice(createInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device);

    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device.getQueue(indices.presentFamily.value(), 0);

    spdlog::info("Created Logical Device");
}

void Device::destroy()
{
    if (device)
    {
        device.destroy(nullptr);
    }
}

auto Device::wait(vk::Fence &fence) -> vk::Result
{
    return device.waitForFences(1, &fence, VK_FALSE, UINT64_MAX);
}

auto Device::reset(vk::Fence &fence) -> vk::Result
{
    return device.resetFences(1, &fence);
}

auto Device::acquireNextImage(vk::SwapchainKHR &swapChain, vk::Semaphore &semaphore, uint32_t &currentBuffer)
    -> vk::Result
{
    return device.acquireNextImageKHR(swapChain, UINT64_MAX, semaphore, nullptr, &currentBuffer);
}

void Device::waitIdle()
{
    device.waitIdle();
}

void Device::update(std::vector<vk::WriteDescriptorSet> &descriptorWrites)
{
    device.updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

auto Device::getSwapchainImages(const vk::SwapchainKHR &swapChain) -> std::vector<vk::Image>
{
    return device.getSwapchainImagesKHR(swapChain);
}

auto Device::create(const vk::CommandBufferAllocateInfo &allocInfo) -> std::vector<vk::CommandBuffer>
{
    return device.allocateCommandBuffers(allocInfo);
}

auto Device::create(const vk::DescriptorSetAllocateInfo &allocInfo) -> std::vector<vk::DescriptorSet>
{
    return device.allocateDescriptorSets(allocInfo);
}

auto Device::create(const vk::CommandPoolCreateInfo &createInfo) -> vk::CommandPool
{
    return device.createCommandPool(createInfo);
}

auto Device::create(const vk::ShaderModuleCreateInfo &createInfo) -> vk::ShaderModule
{
    return device.createShaderModule(createInfo, nullptr);
}

auto Device::create(const vk::DescriptorPoolCreateInfo &createInfo) -> vk::DescriptorPool
{
    return device.createDescriptorPool(createInfo);
}

auto Device::create(const vk::DescriptorSetLayoutCreateInfo &createInfo) -> vk::DescriptorSetLayout
{
    return device.createDescriptorSetLayout(createInfo);
}

auto Device::create(const vk::SamplerCreateInfo &createInfo) -> vk::Sampler
{
    return device.createSampler(createInfo);
}

auto Device::create(const vk::ImageViewCreateInfo &createInfo) -> vk::ImageView
{
    return device.createImageView(createInfo);
}

auto Device::create(const vk::SwapchainCreateInfoKHR &createInfo) -> vk::SwapchainKHR
{
    return device.createSwapchainKHR(createInfo);
}

auto Device::create(const vk::FramebufferCreateInfo &createInfo) -> vk::Framebuffer
{
    return device.createFramebuffer(createInfo);
}

auto Device::create(const vk::FenceCreateInfo &createInfo) -> vk::Fence
{
    return device.createFence(createInfo);
}

auto Device::create(const vk::PipelineLayoutCreateInfo &createInfo) -> vk::PipelineLayout
{
    return device.createPipelineLayout(createInfo);
}

auto Device::create(const vk::GraphicsPipelineCreateInfo &createInfo, vk::PipelineCache cache) -> vk::Pipeline
{
    return device.createGraphicsPipeline(cache, createInfo);
}

auto Device::create(const vk::PipelineCacheCreateInfo &createInfo) -> vk::PipelineCache
{
    return device.createPipelineCache(createInfo);
}

auto Device::create(const vk::RenderPassCreateInfo &createInfo) -> vk::RenderPass
{
    return device.createRenderPass(createInfo);
}

auto Device::create(const vk::SemaphoreCreateInfo &createInfo) -> vk::Semaphore
{
    return device.createSemaphore(createInfo);
}

void Device::destroy(vk::CommandPool pool, std::vector<vk::CommandBuffer> &commandBuffers)
{
    device.freeCommandBuffers(pool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
}

void Device::destroy(vk::CommandPool pool, vk::CommandBuffer commandBuffer)
{
    device.freeCommandBuffers(pool, 1, &commandBuffer);
}

} // namespace tat