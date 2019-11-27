#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{

class Device
{
  public:
    void create();
    void destroy();

    void wait();

    auto wait(vk::Fence &fence) -> vk::Result;
    auto reset(vk::Fence &fence) -> vk::Result;
    auto acquireNextImage(vk::SwapchainKHR &swapChain, vk::Semaphore &semaphore, uint32_t &currentBuffer) -> vk::Result;

    void update(std::vector<vk::WriteDescriptorSet> &descriptorWrites);
    auto getSwapchainImages(const vk::SwapchainKHR &swapChain) -> std::vector<vk::Image>;

    auto create(const vk::CommandBufferAllocateInfo &allocInfo) -> std::vector<vk::CommandBuffer>;
    auto create(const vk::DescriptorSetAllocateInfo &allocInfo) -> std::vector<vk::DescriptorSet>;
    auto create(const vk::CommandPoolCreateInfo &createInfo) -> vk::CommandPool;
    auto create(const vk::ShaderModuleCreateInfo &createInfo) -> vk::ShaderModule;
    auto create(const vk::DescriptorPoolCreateInfo &createInfo) -> vk::DescriptorPool;
    auto create(const vk::DescriptorSetLayoutCreateInfo &createInfo) -> vk::DescriptorSetLayout;
    auto create(const vk::SamplerCreateInfo &createInfo) -> vk::Sampler;
    auto create(const vk::ImageViewCreateInfo &createInfo) -> vk::ImageView;
    auto create(const vk::SwapchainCreateInfoKHR &createInfo) -> vk::SwapchainKHR;
    auto create(const vk::FramebufferCreateInfo &createInfo) -> vk::Framebuffer;
    auto create(const vk::FenceCreateInfo &createInfo) -> vk::Fence;
    auto create(const vk::PipelineLayoutCreateInfo &createInfo) -> vk::PipelineLayout;
    auto create(const vk::GraphicsPipelineCreateInfo &createInfo, vk::PipelineCache cache = nullptr) -> vk::Pipeline;
    auto create(const vk::PipelineCacheCreateInfo &createInfo) -> vk::PipelineCache;
    auto create(const vk::RenderPassCreateInfo &createInfo) -> vk::RenderPass;
    auto create(const vk::SemaphoreCreateInfo &createInfo) -> vk::Semaphore;

    void destroy(vk::CommandPool pool, std::vector<vk::CommandBuffer> &commandBuffers);
    void destroy(vk::CommandPool pool, vk::CommandBuffer commandBuffer);

    template <typename T> void destroy(T t)
    {
        device.destroy(t);
    };

    vk::Device device = nullptr;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

  private:
};

} // namespace tat