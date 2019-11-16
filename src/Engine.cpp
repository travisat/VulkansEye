#include "Engine.hpp"
#include "State.hpp"
#include "vulkan/vulkan.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace tat
{

VkDebugUtilsMessengerEXT debugMessenger;

VKAPI_ATTR auto VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                         VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
                                         const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *
                                         /*pUserData*/) -> VkBool32
{
    std::string prefix;

    if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0)
    {
        spdlog::warn("Validation {}", pCallbackData->pMessage);
    }
    else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0)
    {
        spdlog::error("Validation {}", pCallbackData->pMessage);
    }
    else
    {
        spdlog::info("Validation {}", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

void Engine::init()
{
    spdlog::info("Begin Engine Init");

    auto &state = State::instance();
    createInstance();
    surface.reset(state.window->createSurface(instance.get()));
    pickPhysicalDevice();
    createLogicalDevice();
    createAllocator();

    createSwapChain();
    shadowPass = createShadowPass();
    colorPass = createColorPass();
    createCommandPool();
    createPipelineCache();

    spdlog::info("End Engine Init");
}

void Engine::prepare()
{
    createShadowFramebuffers();
    createColorFramebuffers();
    createSyncObjects();

    createCommandBuffers();
    prepared = true;
    spdlog::info("Engine Prepared");
}

Engine::~Engine()
{
    if (enableValidationLayers)
    {
        if (debugMessenger != nullptr)
        {
            instance->destroyDebugUtilsMessengerEXT(debugMessenger);
        }
    }
    // manually delete
    colorAttachment.reset();
    depthAttachment.reset();
    shadowDepth.reset();

    if (allocator != nullptr)
    {
        vmaDestroyAllocator(allocator);
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
    sunPassInfo.renderPass = shadowPass.get();
    sunPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    sunPassInfo.renderArea.extent.width = settings.at("shadowSize");
    sunPassInfo.renderArea.extent.height = settings.at("shadowSize");
    sunPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    sunPassInfo.pClearValues = clearValues.data();
    sunPassInfo.framebuffer = shadowFbs[currentImage].framebuffer.get();

    commandBuffer.beginRenderPass(sunPassInfo, vk::SubpassContents::eInline);
    state.scene->drawShadow(commandBuffer, currentImage);
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
    scissor.extent = swapChainExtent;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    commandBuffer.setScissor(0, 1, &scissor);

    commandBuffer.setLineWidth(1.0F);

    std::array<vk::ClearValue, 2> clearValues = {};
    clearValues[0].color = std::array<float, 4>{0.0F, 0.0F, 0.0F, 0.0F};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0F, 0};

    vk::RenderPassBeginInfo colorPassInfo = {};
    colorPassInfo.renderPass = colorPass.get();
    colorPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    colorPassInfo.renderArea.extent = swapChainExtent;
    colorPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    colorPassInfo.pClearValues = clearValues.data();
    colorPassInfo.framebuffer = swapChainFbs[currentImage].framebuffer.get();

    commandBuffer.beginRenderPass(colorPassInfo, vk::SubpassContents::eInline);
    state.scene->drawColor(commandBuffer, currentImage);
    if (showOverlay)
    {
        state.overlay->draw(commandBuffer, currentImage);
    }
    commandBuffer.endRenderPass();
}

void Engine::createCommandBuffers()
{
    commandBuffers.resize(swapChainFbs.size());

    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = commandPool.get();
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    commandBuffers = device->allocateCommandBuffersUnique(allocInfo);

    for (uint32_t i = 0; i < commandBuffers.size(); ++i)
    {

        vk::CommandBuffer commandBuffer = commandBuffers[i].get();
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

    if (state.overlay->update)
    {
        state.overlay->recreate();
        createCommandBuffers();
    }

    auto result = device->waitForFences(1, &waitFences[currentImage].get(), VK_FALSE, UINT64_MAX);
    if (result != vk::Result::eSuccess)
    {
        spdlog::error("Unable to wait for fences. Error code {}", result);
        throw std::runtime_error("Unable to wait for fences");
        return;
    }
    result = device->resetFences(1, &waitFences[currentImage].get());
    if (result != vk::Result::eSuccess)
    {
        spdlog::error("Unable to reset fences. Error code {}", result);
        throw std::runtime_error("Unable to reset fences");
        return;
    }

    uint32_t currentBuffer;
    result = device->acquireNextImageKHR(swapChain.get(), UINT64_MAX, presentFinishedSemaphores[currentImage].get(),
                                         nullptr, &currentBuffer);

    if ((result == vk::Result::eErrorOutOfDateKHR) || (result == vk::Result::eSuboptimalKHR))
    {
        resizeWindow();
    }
    else if (result != vk::Result::eSuccess)
    {
        spdlog::error("Unable to draw command buffer. Error code {}", result);
        throw std::runtime_error("Unable to draw command buffer");
        return;
    }

    state.scene->update(currentBuffer, deltaTime);

    const vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo = {};
    submitInfo.pWaitDstStageMask = &waitStages;
    submitInfo.pWaitSemaphores = &presentFinishedSemaphores[currentImage].get();
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentImage].get();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentBuffer].get();
    submitInfo.commandBufferCount = 1;
    graphicsQueue.submit(1, &submitInfo, waitFences[currentImage].get());

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.pNext = nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain.get();
    presentInfo.pImageIndices = &currentBuffer;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentImage].get();
    presentInfo.waitSemaphoreCount = 1;
    result = presentQueue.presentKHR(&presentInfo);

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

    device->waitIdle();

    swapChain.reset();
    swapChainImageViews.clear();
    createSwapChain();
    colorPass.reset();
    colorPass = createColorPass();

    swapChainFbs.clear();
    createColorFramebuffers();
    commandBuffers.clear();
    createCommandBuffers();
    device->waitIdle();

    prepared = true;
}

void Engine::resizeWindow()
{
    auto &state = State::instance();
    if (!prepared)
    {
        return;
    }
    prepared = false;

    int width = 0;
    int height = 0;
    while (width == 0 || height == 0)
    {
        std::tie(width, height) = state.window->getFrameBufferSize();
        state.window->wait();
    }
    device->waitIdle();
    auto &window = state.at("settings").at("window");
    window.at(0) = width;
    window.at(1) = height;

    swapChain.reset();
    swapChainImageViews.clear();
    createSwapChain();
    colorPass.reset();
    colorPass = createColorPass();

    state.scene->recreate();
    state.overlay->recreate();

    swapChainFbs.clear();
    createColorFramebuffers();

    commandBuffers.clear();
    createCommandBuffers();
    device->waitIdle();

    prepared = true;
    spdlog::info("Resized window to {}x{}", width, height);
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

    auto extensions = getRequiredExtensions();

    vk::InstanceCreateInfo createInfo = {};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    vk::DynamicLoader dl;
    auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    instance = vk::createInstanceUnique(createInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

    spdlog::info("Created Vulkan instance");
}

void Engine::pickPhysicalDevice()
{
    auto devs = instance->enumeratePhysicalDevices();

    assert(!devs.empty());

    for (const auto &dev : devs)
    {
        if (isDeviceSuitable(dev))
        {
            properties = dev.getProperties();
            spdlog::info("Device: {}", properties.deviceName);
            spdlog::info("ID: {}", properties.deviceID);
            spdlog::info("Type: {}", properties.deviceType);
            spdlog::info("Driver: {}", properties.driverVersion);
            spdlog::info("API: {}", properties.apiVersion);
            physicalDevice = dev;
            msaaSamples = getMaxUsableSampleCount();
            return;
        }
    }

    if (!physicalDevice)
    {
        spdlog::error("Unable to find suitable physical device");
        throw std::runtime_error("Unable to find suitable physical device");
    }
}

auto Engine::isDeviceSuitable(vk::PhysicalDevice const &device) -> bool
{
    QueueFamilyIndices indicies = findQueueFamiles(device);

    bool extensionsSupported = checkDeviceExtensionsSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    auto supportedFeatures = device.getFeatures();

    return indicies.isComplete() && extensionsSupported && swapChainAdequate &&
           (supportedFeatures.samplerAnisotropy != VK_FALSE) && (supportedFeatures.tessellationShader != VK_FALSE) &&
           (supportedFeatures.geometryShader != VK_FALSE);
};

auto Engine::findQueueFamiles(vk::PhysicalDevice const &device) -> QueueFamilyIndices
{
    QueueFamilyIndices indices;

    auto queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
        {
            indices.graphicsFamily = i;
        }

        auto presentSupport = device.getSurfaceSupportKHR(i, surface.get());

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

auto Engine::querySwapChainSupport(vk::PhysicalDevice const &device) -> SwapChainSupportDetails
{
    SwapChainSupportDetails details{};
    details.capabilities = device.getSurfaceCapabilitiesKHR(surface.get());
    details.formats = device.getSurfaceFormatsKHR(surface.get());
    details.presentModes = device.getSurfacePresentModesKHR(surface.get());
    return details;
}

auto Engine::checkDeviceExtensionsSupport(vk::PhysicalDevice const &device) -> bool
{
    auto extensions = device.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : extensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void Engine::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamiles(physicalDevice);

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
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.enabledLayerCount = 0;
    if (enableValidationLayers)
    {
        vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
        debugInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        debugInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        debugInfo.pfnUserCallback = debugCallback;

        debugMessenger = instance->createDebugUtilsMessengerEXT(debugInfo);
    }

    device = physicalDevice.createDeviceUnique(createInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device.get());

    graphicsQueue = device->getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device->getQueue(indices.presentFamily.value(), 0);
    spdlog::info("Created Logical Device");
}

void Engine::createAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device.get();

    auto result = vmaCreateAllocator(&allocatorInfo, &allocator);
    if (result != VK_SUCCESS)
    {
        spdlog::error("Unable to create Memory Allocator. Error code {}", result);
        throw std::runtime_error("Unable to create Memory Allocator");
        return;
    }
    spdlog::info("Created Memory Allocator");
}

void Engine::createSwapChain()
{
    auto &state = State::instance();
    auto &window = state.at("settings").at("window");
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window.at(0), window.at(1));

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.surface = surface.get();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    QueueFamilyIndices indices = findQueueFamiles(physicalDevice);
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

    swapChain = device->createSwapchainKHRUnique(createInfo);

    swapChainImages = device->getSwapchainImagesKHR(swapChain.get());
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = swapChainImages[i];
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = swapChainImageFormat;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        swapChainImageViews[i] = device->createImageViewUnique(viewInfo);
    }
    spdlog::info("Created SwapChain");
}

void Engine::createShadowFramebuffers()
{
    auto &state = State::instance();
    auto &settings = state.at("settings");
    shadowDepth = std::make_unique<Image>();
    shadowDepth->imageInfo.format = findDepthFormat();
    shadowDepth->imageInfo.usage =
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc;
    shadowDepth->memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    shadowDepth->imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    shadowDepth->resize(settings.at("shadowSize"), settings.at("shadowSize"));
    shadowDepth->transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    shadowFbs.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        shadowFbs[i].renderPass = shadowPass.get();
        shadowFbs[i].width = settings.at("shadowSize");
        shadowFbs[i].height = settings.at("shadowSize");
        shadowFbs[i].attachments = {state.scene->shadow->imageView.get(), shadowDepth->imageView.get()};
        shadowFbs[i].create();
    }
    spdlog::info("Created Framebuffer for shadows");
}

void Engine::createColorFramebuffers()
{
    colorAttachment = std::make_unique<Image>();
    colorAttachment->imageInfo.format = swapChainImageFormat;
    colorAttachment->imageInfo.samples = msaaSamples;
    colorAttachment->imageInfo.usage =
        vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
    colorAttachment->memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    colorAttachment->resize(swapChainExtent.width, swapChainExtent.height);
    colorAttachment->transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

    depthAttachment = std::make_unique<Image>();
    depthAttachment->imageInfo.format = findDepthFormat();
    depthAttachment->imageInfo.samples = msaaSamples;
    depthAttachment->imageInfo.usage =
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc;
    depthAttachment->memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    depthAttachment->imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    depthAttachment->resize(swapChainExtent.width, swapChainExtent.height);
    depthAttachment->transitionImageLayout(vk::ImageLayout::eUndefined,
                                           vk::ImageLayout::eDepthStencilAttachmentOptimal);
    swapChainFbs.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        swapChainFbs[i].renderPass = colorPass.get();
        swapChainFbs[i].width = swapChainExtent.width;
        swapChainFbs[i].height = swapChainExtent.height;
        swapChainFbs[i].attachments = {colorAttachment->imageView.get(), depthAttachment->imageView.get(),
                                       swapChainImageViews[i].get()};
        swapChainFbs[i].create();
    }
    spdlog::info("Created Framebuffer for display");
}

void Engine::createCommandPool()
{
    QueueFamilyIndices QueueFamilyIndices = findQueueFamiles(physicalDevice);

    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.queueFamilyIndex = QueueFamilyIndices.graphicsFamily.value();

    commandPool = device->createCommandPoolUnique(poolInfo);
    spdlog::info("Created Command Pool");
}

void Engine::createPipelineCache()
{
    vk::PipelineCacheCreateInfo createInfo{};
    pipelineCache = device->createPipelineCacheUnique(createInfo);
    spdlog::info("Created Pipeline Cache");
}

void Engine::createSyncObjects()
{
    presentFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    waitFences.resize(MAX_FRAMES_IN_FLIGHT);

    for (auto &semaphore : presentFinishedSemaphores)
    {
        semaphore = device->createSemaphoreUnique({});
    }
    for (auto &semaphore : renderFinishedSemaphores)
    {
        semaphore = device->createSemaphoreUnique({});
    }
    for (auto &fence : waitFences)
    {
        vk::FenceCreateInfo fenceInfo = {};
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        fence = device->createFenceUnique(fenceInfo);
    }
    spdlog::info("Created Sync Objects");
}

auto Engine::beginSingleTimeCommands() -> vk::CommandBuffer
{
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool.get();
    allocInfo.commandBufferCount = 1;

    auto commandBuffer = device->allocateCommandBuffers(allocInfo);

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

    graphicsQueue.submit(submitInfo, nullptr);
    graphicsQueue.waitIdle();
    device->freeCommandBuffers(commandPool.get(), 1, &commandBuffer);
};

auto Engine::createShaderModule(const std::string &filename) -> vk::UniqueShaderModule
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

    return device->createShaderModuleUnique(createInfo, nullptr);
}

auto Engine::getRequiredExtensions() -> std::vector<const char *>
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
};

auto Engine::getMaxUsableSampleCount() -> vk::SampleCountFlagBits
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

auto Engine::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) -> vk::SurfaceFormatKHR
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

auto Engine::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) -> vk::PresentModeKHR
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == defaultPresentMode)
        {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

auto Engine::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, uint32_t windowWidth,
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

auto Engine::findDepthFormat() -> vk::Format
{
    std::vector<vk::Format> candidates = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
                                          vk::Format::eD24UnormS8Uint};

    for (vk::Format format : candidates)
    {
        auto props = physicalDevice.getFormatProperties(format);
        if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported depth format");
}

} // namespace tat