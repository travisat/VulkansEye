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
    if constexpr (Debug::enableValidationLayers)
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

    spdlog::info("Created Engine");
}

void Engine::prepare()
{
    createShadowFramebuffers();
    createColorFramebuffers();
    presentSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    waitFences.resize(MAX_FRAMES_IN_FLIGHT);

    createCommandBuffers();
    prepared = true;
    spdlog::info("Prepared Engine");
}

void Engine::destroy()
{
    // manually delete
    colorAttachment.destroy();
    depthAttachment.destroy();
    shadowDepth.destroy();

    if constexpr (Debug::enableValidationLayers)
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

    for (auto &frameBuffer : colorFramebuffers)
    {
        frameBuffer.destroy();
    }
    for (auto &frameBuffer : shadowFramebuffers)
    {
        frameBuffer.destroy();
    }

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

    spdlog::info("Destroyed Engine");
}

void Engine::renderShadows(vk::CommandBuffer commandBuffer, int32_t currentImage)
{
    auto &state = State::instance();
    auto &settings = state.at("settings");
    vk::Viewport viewport{};
    viewport.width = settings.at("shadowSize");
    viewport.height = settings.at("shadowSize");
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor{};
    scissor.extent.width = settings.at("shadowSize");
    scissor.extent.height = settings.at("shadowSize");
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    commandBuffer.setScissor(0, 1, &scissor);

    commandBuffer.setLineWidth(1.0F);

    std::array<vk::ClearValue, 2> clearValues = {};
    clearValues[0].color = std::array<float, 4>{0.F, 0.F, 0.F, 0.F};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0F, 0};

    vk::RenderPassBeginInfo sunPassInfo = {};
    sunPassInfo.renderPass = shadowPass.renderPass;
    sunPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    sunPassInfo.renderArea.extent.width = settings.at("shadowSize");
    sunPassInfo.renderArea.extent.height = settings.at("shadowSize");
    sunPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    sunPassInfo.pClearValues = clearValues.data();
    sunPassInfo.framebuffer = shadowFramebuffers[currentImage].framebuffer;

    commandBuffer.beginRenderPass(sunPassInfo, vk::SubpassContents::eInline);
    state.scene.drawShadow(commandBuffer, currentImage);
    commandBuffer.endRenderPass();
}

void Engine::renderColors(vk::CommandBuffer commandBuffer, int32_t currentImage)
{
    auto &state = State::instance();
    auto &settings = state.at("settings");
    vk::Viewport viewport{};
    viewport.width = settings.at("window").at(0);
    viewport.height = settings.at("window").at(1);
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor{};
    scissor.extent = swapChain.extent;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    commandBuffer.setScissor(0, 1, &scissor);

    commandBuffer.setLineWidth(1.0F);

    std::array<vk::ClearValue, 2> clearValues = {};
    clearValues[0].color = std::array<float, 4>{0.0F, 0.0F, 0.0F, 0.0F};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0F, 0};

    vk::RenderPassBeginInfo colorPassInfo = {};
    colorPassInfo.renderPass = colorPass.renderPass;
    colorPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    colorPassInfo.renderArea.extent = swapChain.extent;
    colorPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    colorPassInfo.pClearValues = clearValues.data();
    colorPassInfo.framebuffer = colorFramebuffers[currentImage].framebuffer;

    commandBuffer.beginRenderPass(colorPassInfo, vk::SubpassContents::eInline);
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

    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    commandBuffers = device.create(allocInfo);

    for (uint32_t i = 0; i < commandBuffers.size(); ++i)
    {

        vk::CommandBuffer commandBuffer = commandBuffers[i];
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
    if (!prepared)
    {
        return;
    }

    if (state.overlay.update)
    {
        state.overlay.cleanup();
        state.overlay.recreate();
        createCommandBuffers();
    }

    auto result = device.wait(waitFences[currentImage].fence);
    if (result != vk::Result::eSuccess)
    {
        spdlog::error("Unable to wait for fences. Error code {}", result);
        throw std::runtime_error("Unable to wait for fences");
        return;
    }
    result = device.reset(waitFences[currentImage].fence);
    if (result != vk::Result::eSuccess)
    {
        spdlog::error("Unable to reset fences. Error code {}", result);
        throw std::runtime_error("Unable to reset fences");
        return;
    }

    uint32_t currentBuffer;
    result = device.acquireNextImage(swapChain.swapChain, presentSemaphores[currentImage].semaphore, currentBuffer);

    if ((result == vk::Result::eErrorOutOfDateKHR) || (result == vk::Result::eSuboptimalKHR))
    {
        resizeWindow();
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
    vk::SubmitInfo submitInfo = {};
    submitInfo.pWaitDstStageMask = &waitStages;
    submitInfo.pWaitSemaphores = &presentSemaphores[currentImage].semaphore;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderSemaphores[currentImage].semaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentBuffer];
    submitInfo.commandBufferCount = 1;
    device.graphicsQueue.submit(1, &submitInfo, waitFences[currentImage].fence);

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.pNext = nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain.swapChain;
    presentInfo.pImageIndices = &currentBuffer;
    presentInfo.pWaitSemaphores = &renderSemaphores[currentImage].semaphore;
    presentInfo.waitSemaphoreCount = 1;
    result = device.presentQueue.presentKHR(&presentInfo);

    if (result == vk::Result::eErrorOutOfDateKHR)
    {
        resizeWindow();
        return;
    }
    if (result != vk::Result::eSuccess)
    {
        spdlog::error("Unable to present command buffer. Error code {}", result);
        throw std::runtime_error("Unable to present command buffer");
        return;
    }
    if (updateCommandBuffer)
    {
        updateWindow();
        updateCommandBuffer = false;
    }

    currentImage = (currentImage + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Engine::updateWindow()
{
    if (!prepared)
    {
        return;
    }
    prepared = false;

    device.waitIdle();

    // Steps to update
    // 1: free commandBuffers
    device.destroy(commandPool, commandBuffers);
    // 2: destroy color framebuffers
    for (auto &frameBuffer : colorFramebuffers)
    {
        frameBuffer.destroy();
    }
    // 3: destroy color renderpass
    colorPass.destroy();
    // 4: destroy swapchain
    swapChain.destroy();
    // 5: create swap chain
    swapChain.create();
    // 6: create color renderpass
    colorPass.create();
    // 7: create color framebuffers
    createColorFramebuffers();
    // 8: create commandbuffers
    createCommandBuffers();

    device.waitIdle();

    prepared = true;
}

void Engine::resizeWindow()
{
    if (!prepared)
    {
        return;
    }
    prepared = false;

    auto &state = State::instance();

    state.window.resize();

    // Steps to resize
    // 1: free commandBuffers
    device.destroy(commandPool, commandBuffers);
    // 2: destroy color framebuffers
    for (auto &frameBuffer : colorFramebuffers)
    {
        frameBuffer.destroy();
    }
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

    device.waitIdle();

    prepared = true;
    spdlog::info("Resized window to {}x{}", state.window.width, state.window.height);
}

void Engine::createInstance()
{
    auto &state = State::instance();
    vk::ApplicationInfo appInfo = {};
    appInfo.pApplicationName = state.at("settings").at("name").get<std::string>().c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "vulcanned";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if constexpr (Debug::enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    vk::InstanceCreateInfo createInfo = {};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;

    if constexpr (Debug::enableValidationLayers)
    {
        createInfo.enabledLayerCount = debug.validationLayers.size();
        createInfo.ppEnabledLayerNames = debug.validationLayers.data();
    }

    vk::DynamicLoader dl;
    auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    instance = vk::createInstance(createInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

    spdlog::info("Created Vulkan instance");
}

void Engine::createShadowFramebuffers()
{
    auto &state = State::instance();
    auto &settings = state.at("settings");
    shadowDepth.imageInfo.format = findDepthFormat();
    shadowDepth.imageInfo.usage =
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc;
    shadowDepth.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    shadowDepth.imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    shadowDepth.resize(settings.at("shadowSize"), settings.at("shadowSize"));
    shadowDepth.transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    Debug::setName(device.device, shadowDepth.image, "ShadowDepth");

    shadowFramebuffers.resize(swapChain.count);
    for (size_t i = 0; i < swapChain.count; i++)
    {
        shadowFramebuffers[i].renderPass = shadowPass.renderPass;
        shadowFramebuffers[i].width = settings.at("shadowSize");
        shadowFramebuffers[i].height = settings.at("shadowSize");
        shadowFramebuffers[i].attachments = {state.scene.shadow.imageView, shadowDepth.imageView};
        shadowFramebuffers[i].create();
    }
    spdlog::info("Created Framebuffer for shadows");
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
    spdlog::info("Created Framebuffer for display");
}

void Engine::createCommandPool()
{
    QueueFamilyIndices QueueFamilyIndices = SwapChain::findQueueFamiles(physicalDevice.device);

    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.queueFamilyIndex = QueueFamilyIndices.graphicsFamily.value();

    commandPool = device.create(poolInfo);
    spdlog::info("Created Command Pool");
}

auto Engine::beginSingleTimeCommands() -> vk::CommandBuffer
{
    vk::CommandBufferAllocateInfo allocInfo = {};
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

    vk::SubmitInfo submitInfo = {};
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
        throw std::runtime_error("failed to open file");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    vk::ShaderModuleCreateInfo createInfo = {};
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(buffer.data());

    return device.create(createInfo);
}

auto Engine::findDepthFormat() -> vk::Format
{
    std::vector<vk::Format> candidates = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
                                          vk::Format::eD24UnormS8Uint};

    for (vk::Format format : candidates)
    {
        auto properties = physicalDevice.getFormatProperties(format);
        if (properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported depth format");
}

} // namespace tat