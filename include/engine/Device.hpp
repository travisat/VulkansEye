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

    void waitIdle();

    auto waitForFences(vk::Fence &fence) -> vk::Result;
    auto resetFences(vk::Fence &fence) -> vk::Result;
    auto acquireNextImage(vk::SwapchainKHR &swapChain, vk::Semaphore &semaphore, uint32_t &currentBuffer) -> vk::Result;

    auto allocateCommandBuffers(const vk::CommandBufferAllocateInfo &allocInfo) -> std::vector<vk::CommandBuffer>;
    auto allocateDescriptorSets(const vk::DescriptorSetAllocateInfo &allocInfo) -> std::vector<vk::DescriptorSet>;
    void updateDescriptorSets(std::vector<vk::WriteDescriptorSet> &descriptorWrites);
    auto getSwapchainImages(const vk::SwapchainKHR &swapChain) -> std::vector<vk::Image>;

    auto createCommandPool(const vk::CommandPoolCreateInfo &createInfo) -> vk::CommandPool;
    auto createShaderModule(const vk::ShaderModuleCreateInfo &createInfo) -> vk::ShaderModule;
    auto createDescriptorPool(const vk::DescriptorPoolCreateInfo &createInfo) -> vk::DescriptorPool;
    auto createDescriptorSetLayout(const vk::DescriptorSetLayoutCreateInfo &createInfo) -> vk::DescriptorSetLayout;
    auto createSampler(const vk::SamplerCreateInfo &createInfo) -> vk::Sampler;
    auto createImageView(const vk::ImageViewCreateInfo &createInfo) -> vk::ImageView;
    auto createSwapchain(const vk::SwapchainCreateInfoKHR &createInfo) -> vk::SwapchainKHR;
    auto createFramebuffer(const vk::FramebufferCreateInfo &createInfo) -> vk::Framebuffer;
    auto createFence(const vk::FenceCreateInfo &createInfo) -> vk::Fence;
    auto createPipelineLayout(const vk::PipelineLayoutCreateInfo &createInfo) -> vk::PipelineLayout;
    auto createGraphicsPipeline(const vk::GraphicsPipelineCreateInfo &createInfo, vk::PipelineCache cache = nullptr) -> vk::Pipeline;
    auto createPipelineCache(const vk::PipelineCacheCreateInfo &createInfo) -> vk::PipelineCache;
    auto createRenderPass(const vk::RenderPassCreateInfo &createInfo) -> vk::RenderPass;
    auto createSemaphore() -> vk::Semaphore;

    void destroyCommandBuffers(vk::CommandPool pool, std::vector<vk::CommandBuffer> &commandBuffers);
    void destroyCommandBuffer(vk::CommandPool pool, vk::CommandBuffer commandBuffer);
    
    template <typename T>
    void destroy(T t)
    {
      device.destroy(t);
    };

    vk::Device device = nullptr;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

  private:
};

} // namespace tat