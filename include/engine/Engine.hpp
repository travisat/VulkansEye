#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <vk_mem_alloc.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "engine/Allocator.hpp"
#include "engine/Debug.hpp"
#include "engine/Fence.hpp"
#include "engine/Framebuffer.hpp"
#include "engine/PipelineCache.hpp"
#include "engine/RenderPass.hpp"
#include "engine/Semaphore.hpp"
#include "engine/SwapChain.hpp"
#include "engine/PhysicalDevice.hpp"

#include "Image.hpp"

namespace tat
{

const int MAX_FRAMES_IN_FLIGHT = 2;

class Engine
{
  public:
    void create();
    void prepare();
    void destroy();
    void drawFrame(float deltaTime);

    bool showOverlay = false;
    bool updateCommandBuffer = false;

    vk::Instance instance;
    vk::SurfaceKHR surface;
    vk::Device device;
    PhysicalDevice physicalDevice;

    Allocator allocator{};
    
    SwapChain swapChain;

    PipelineCache pipelineCache;
    RenderPass colorPass;
    RenderPass shadowPass;

    vk::PresentModeKHR defaultPresentMode = vk::PresentModeKHR::eMailbox;

    auto beginSingleTimeCommands() -> vk::CommandBuffer;
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

    auto createShaderModule(const std::string &filename) -> vk::ShaderModule;
    auto findDepthFormat() -> vk::Format;

  private:
    std::vector<Framebuffer> shadowFramebuffers{};
    Image shadowDepth;
    std::vector<Framebuffer> colorFramebuffers{};
    Image colorAttachment;
    Image depthAttachment;

    std::vector<vk::CommandBuffer> commandBuffers{};
   
    std::vector<Semaphore> presentSemaphores{};
    std::vector<Semaphore> renderSemaphores{};
    std::vector<Fence> waitFences{};

    Debug debug;

    vk::CommandPool commandPool;
    

    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

    int32_t currentImage = 0;
    bool prepared = false;

    void createCommandBuffers();

    void renderShadows(vk::CommandBuffer commandBuffer, int32_t currentImage);
    void renderColors(vk::CommandBuffer commandBuffer, int32_t currentImage);

    void updateWindow();
    void resizeWindow();

    void createInstance();
    void createLogicalDevice();
    void createColorFramebuffers();
    void createShadowFramebuffers();
    void createCommandPool();
    void createPipelineCache();
   
    static auto chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
        -> vk::SurfaceFormatKHR;
   
};

} // namespace tat