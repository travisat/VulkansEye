#include "engine/Engine.hpp"
#include "State.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

#include <spdlog/spdlog.h>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace tat
{

void Engine::create()
{
    auto &state = State::instance();
    createInstance();
    if constexpr (Debug::enable)
    {
        debug.create(&instance);
    }
    surface = state.window.createSurface(instance);
    physicalDevice.pick(instance);

    device.create();

    allocator.create(physicalDevice.device, device.device);
    swapChain.create();
    shadowPass.loadShadow();
    shadowPass.create();
    colorPass.loadColor();
    colorPass.create();
    createCommandPool();
    pipelineCache.create();

    if constexpr (Debug::enable)
    {
        spdlog::info("Created Engine");
    }
}

void Engine::prepare()
{
    createShadowFramebuffers();
    createColorFramebuffers();
    presentSemaphores.resize(maxFramesInFlight);
    renderSemaphores.resize(maxFramesInFlight);
    waitFences.resize(maxFramesInFlight);

    createCommandBuffers();
    prepared = true;

    if constexpr (Debug::enable)
    {
        spdlog::info("Prepared Engine");
    }
}

void Engine::destroy()
{
    // manually destroy
    colorAttachment.destroy();
    depthAttachment.destroy();
    shadowDepth.destroy();

    if constexpr (Debug::enable)
    {
        debug.destroy();
    }

    if (!commandBuffers.empty())
    {
        device.destroy(commandPool, commandBuffers);
    }
    if (commandPool)
    {
        device.destroy(commandPool);
    }

    renderSemaphores.clear();
    presentSemaphores.clear();
    waitFences.clear();
    colorPass.destroy();
    shadowPass.destroy();
    swapChain.destroy();
    pipelineCache.destroy();

    colorFramebuffers.clear();
    shadowFramebuffers.clear();

    allocator.destroy();
    device.destroy();

    if (surface)
    {
        instance.destroySurfaceKHR(surface);
    }
    if (instance)
    {
        instance.destroy();
    }

    if constexpr (Debug::enable)
    {
        spdlog::info("Destroyed Engine");
    }
}

void Engine::renderShadows(vk::CommandBuffer commandBuffer, int32_t currentImage)
{
    auto &state = State::instance();
    vk::Viewport viewport{};
    viewport.width = state.scene.shadowSize;
    viewport.height = state.scene.shadowSize;
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor{};
    scissor.extent.width = state.scene.shadowSize;
    scissor.extent.height = state.scene.shadowSize;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    commandBuffer.setScissor(0, 1, &scissor);

    commandBuffer.setLineWidth(1.0F);

    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].color = std::array<float, 4>{0.F, 0.F, 0.F, 0.F};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0F, 0};

    vk::RenderPassBeginInfo shadowPassBeginInfo{};
    shadowPassBeginInfo.renderPass = shadowPass.renderPass;
    shadowPassBeginInfo.renderArea.offset = vk::Offset2D{0, 0};
    shadowPassBeginInfo.renderArea.extent.width = state.scene.shadowSize;
    shadowPassBeginInfo.renderArea.extent.height = state.scene.shadowSize;
    shadowPassBeginInfo.clearValueCount = clearValues.size();
    shadowPassBeginInfo.pClearValues = clearValues.data();
    shadowPassBeginInfo.framebuffer = shadowFramebuffers[currentImage].framebuffer;

    commandBuffer.beginRenderPass(shadowPassBeginInfo, vk::SubpassContents::eInline);
    state.scene.drawShadow(commandBuffer, currentImage);
    commandBuffer.endRenderPass();
}

void Engine::renderColors(vk::CommandBuffer commandBuffer, int32_t currentImage)
{
    auto &state = State::instance();
    vk::Viewport viewport{};
    viewport.width = state.window.width;
    viewport.height = state.window.height;
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor{};
    scissor.extent = swapChain.extent;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    commandBuffer.setScissor(0, 1, &scissor);

    commandBuffer.setLineWidth(1.0F);

    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].color = std::array<float, 4>{0.0F, 0.0F, 0.0F, 0.0F};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0F, 0};

    vk::RenderPassBeginInfo colorPassBeginInfo{};
    colorPassBeginInfo.renderPass = colorPass.renderPass;
    colorPassBeginInfo.renderArea.offset = vk::Offset2D{0, 0};
    colorPassBeginInfo.renderArea.extent = swapChain.extent;
    colorPassBeginInfo.clearValueCount = clearValues.size();
    colorPassBeginInfo.pClearValues = clearValues.data();
    colorPassBeginInfo.framebuffer = colorFramebuffers[currentImage].framebuffer;

    commandBuffer.beginRenderPass(colorPassBeginInfo, vk::SubpassContents::eInline);
    state.scene.drawColor(commandBuffer, currentImage);
    if (showOverlay)
    {
        state.overlay.draw(commandBuffer, currentImage);
    }
    commandBuffer.endRenderPass();
}

void Engine::createCommandBuffers()
{
    commandBuffers.resize(colorFramebuffers.size());

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = commandBuffers.size();
    commandBuffers = device.create(allocInfo);

    for (uint32_t i = 0; i < commandBuffers.size(); ++i)
    {
        auto commandBuffer = commandBuffers[i];
        vk::CommandBufferBeginInfo beginInfo{};
        commandBuffer.begin(beginInfo);
        // draw shadows
        renderShadows(commandBuffer, i);
        // draw colors
        renderColors(commandBuffer, i);
        commandBuffer.end();
    }
}

void Engine::drawFrame(float deltaTime)
{
    auto &state = State::instance();

    if (showOverlay || updateCommandBuffer)
    {
        updateCommandBuffers();
        updateCommandBuffer = false;
    }

    if (device.wait(waitFences[currentImage].fence) != vk::Result::eSuccess)
    {
        spdlog::error("Unable to wait for fences");
        throw std::runtime_error("Unable to wait for fences");
        return;
    }
    if (device.reset(waitFences[currentImage].fence) != vk::Result::eSuccess)
    {
        spdlog::error("Unable to reset fences");
        throw std::runtime_error("Unable to reset fences");
        return;
    }

    uint32_t currentBuffer;
    auto result =
        device.acquireNextImage(swapChain.swapChain, presentSemaphores[currentImage].semaphore, currentBuffer);

    if ((result == vk::Result::eErrorOutOfDateKHR) || (result == vk::Result::eSuboptimalKHR))
    {
        updateCommandBuffers();
        return;
    }
    if (result != vk::Result::eSuccess)
    {
        spdlog::error("Unable to draw command buffer. Error code {}", result);
        throw std::runtime_error("Unable to draw command buffer");
        return;
    }

    state.scene.update(currentBuffer, deltaTime);

    const vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo{};
    submitInfo.pWaitDstStageMask = &waitStages;
    submitInfo.pWaitSemaphores = &presentSemaphores[currentImage].semaphore;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderSemaphores[currentImage].semaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentBuffer];
    submitInfo.commandBufferCount = 1;
    device.graphicsQueue.submit(1, &submitInfo, waitFences[currentImage].fence);

    vk::PresentInfoKHR presentInfo{};
    presentInfo.pNext = nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain.swapChain;
    presentInfo.pImageIndices = &currentBuffer;
    presentInfo.pWaitSemaphores = &renderSemaphores[currentImage].semaphore;
    presentInfo.waitSemaphoreCount = 1;
    result = device.presentQueue.presentKHR(&presentInfo);

    if (result == vk::Result::eErrorOutOfDateKHR)
    {
        updateCommandBuffers();
        return;
    }
    if (result != vk::Result::eSuccess)
    {
        spdlog::error("Unable to present command buffer. Error code {}", result);
        throw std::runtime_error("Unable to present command buffer");
        return;
    }

    currentImage = (currentImage + 1) % maxFramesInFlight;
}

void Engine::updateCommandBuffers()
{
    device.wait();
    device.destroy(commandPool, commandBuffers);
    createCommandBuffers();
    device.wait();
}

void Engine::resize(int width, int height)
{
    if (!prepared)
    {
        return;
    }

    auto &state = State::instance();
    state.window.resize(width, height);

    device.wait();

    // Steps to resize
    // 1: free commandBuffers
    device.destroy(commandPool, commandBuffers);
    // 2: destroy color framebuffers
    colorFramebuffers.clear();
    // 3: destroy color renderpass
    colorPass.destroy();
    // 4: destroy swapchain
    swapChain.destroy();
    // 5: cleanup scene
    state.scene.cleanup();
    // 6: cleanup overlay
    state.overlay.cleanup();
    // 7: create swap chain
    swapChain.create();
    // 8: create color renderpass
    colorPass.create();
    // 9: recreate scene
    state.scene.recreate();
    // 10: recreate overlay
    state.overlay.recreate();
    // 11: create color framebuffers
    createColorFramebuffers();
    // 12: create commandbuffers
    createCommandBuffers();

    device.wait();

    if constexpr (Debug::enable)
    {
        spdlog::info("Resized window to {}x{}", state.window.width, state.window.height);
    }
}

void Engine::createInstance()
{
    vk::ApplicationInfo appInfo{};
    appInfo.pApplicationName = "Vulkans Eye";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "vulcanned";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if constexpr (Debug::enable)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    vk::InstanceCreateInfo createInfo{};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;

    if constexpr (Debug::enable)
    {
        createInfo.enabledLayerCount = debug.validationLayers.size();
        createInfo.ppEnabledLayerNames = debug.validationLayers.data();
    }

    vk::DynamicLoader dl;
    auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    instance = vk::createInstance(createInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

    if constexpr (Debug::enable)
    {
        spdlog::info("Created Vulkan instance");
    }
}

void Engine::createShadowFramebuffers()
{
    auto &state = State::instance();
    shadowDepth.imageInfo.format = findDepthFormat();
    shadowDepth.imageInfo.usage =
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc;
    shadowDepth.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    shadowDepth.imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    shadowDepth.resize(floor(state.scene.shadowSize), floor(state.scene.shadowSize));
    shadowDepth.transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    if constexpr (Debug::enable)
    {
        Debug::setName(device.device, shadowDepth.image, "ShadowDepth");
    }

    shadowFramebuffers.resize(swapChain.count);
    for (auto &framebuffer : shadowFramebuffers)
    {
        framebuffer.renderPass = shadowPass.renderPass;
        framebuffer.width = floor(state.scene.shadowSize);
        framebuffer.height = floor(state.scene.shadowSize);
        framebuffer.attachments = {state.scene.shadow.imageView, shadowDepth.imageView};
        framebuffer.create();
    }

    if constexpr (Debug::enable)
    {
        spdlog::info("Created Framebuffer for shadows");
    }
}

void Engine::createColorFramebuffers()
{
    colorAttachment.imageInfo.format = swapChain.format;
    colorAttachment.imageInfo.samples = physicalDevice.msaaSamples;
    colorAttachment.imageInfo.usage =
        vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
    colorAttachment.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    colorAttachment.resize(swapChain.extent);
    colorAttachment.transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

    depthAttachment.imageInfo.format = findDepthFormat();
    depthAttachment.imageInfo.samples = physicalDevice.msaaSamples;
    depthAttachment.imageInfo.usage =
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc;
    depthAttachment.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    depthAttachment.imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    depthAttachment.resize(swapChain.extent);
    depthAttachment.transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    colorFramebuffers.resize(swapChain.count);
    for (size_t i = 0; i < swapChain.count; i++)
    {
        colorFramebuffers[i].renderPass = colorPass.renderPass;
        colorFramebuffers[i].width = swapChain.extent.width;
        colorFramebuffers[i].height = swapChain.extent.height;
        colorFramebuffers[i].attachments = {colorAttachment.imageView, depthAttachment.imageView,
                                            swapChain.imageViews[i]};
        colorFramebuffers[i].create();
    }

    if constexpr (Debug::enable)
    {
        spdlog::info("Created Framebuffer for display");
    }
}

void Engine::createCommandPool()
{
    auto QueueFamilyIndices = SwapChain::findQueueFamiles(physicalDevice.device);

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.queueFamilyIndex = QueueFamilyIndices.graphicsFamily.value();

    commandPool = device.create(poolInfo);

    if constexpr (Debug::enable)
    {
        spdlog::info("Created Command Pool");
    }
}

auto Engine::beginSingleTimeCommands() -> vk::CommandBuffer
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    auto commandBuffer = device.create(allocInfo);

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    commandBuffer[0].begin(beginInfo);

    return commandBuffer[0];
};

void Engine::endSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    device.graphicsQueue.submit(submitInfo, nullptr);
    device.graphicsQueue.waitIdle();
    device.destroy(commandPool, commandBuffer);
};

auto Engine::createShaderModule(const std::string &filename) -> vk::ShaderModule
{
    if (!std::filesystem::exists(filename))
    {
        spdlog::error("Shader {} does not exist", filename);
        throw std::runtime_error("Shader does not exist");
    }

    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        spdlog::error("Failed to open ", filename);
        throw std::runtime_error("failed to open file");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(buffer.data());

    return device.create(createInfo);
}

auto Engine::findDepthFormat() -> vk::Format
{
    std::vector<vk::Format> candidates = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
                                          vk::Format::eD24UnormS8Uint};
    for (auto &format : candidates)
    {
        if (physicalDevice.getFormatProperties(format).optimalTilingFeatures &
            vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported depth format");
}

} // namespace tat