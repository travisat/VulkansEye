#include "Engine.hpp"
#include "helpers.hpp"
#include <cstdint>

namespace tat
{

VkDebugUtilsMessengerEXT debugMessenger;

VKAPI_ATTR auto VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                         VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
                                         const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *
                                         /*pUserData*/) -> VkBool32
{
    std::string prefix;
    auto logger = spdlog::get("debugLogger");

    if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0)
    {
        logger->warn("Validation {}", pCallbackData->pMessage);
    }
    else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0)
    {
        logger->error("Validation {}", pCallbackData->pMessage);
    }
    else
    {
        logger->info("Validation {}", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

void Engine::init()
{
    debugLogger = spdlog::get("debugLogger");
    createInstance();
    vulkan->surface = vulkan->window->createSurface(vulkan->instance);
    pickPhysicalDevice();
    createLogicalDevice();
    createAllocator();

    createSwapChain();
    vulkan->shadowPass = createShadowPass(vulkan);
    vulkan->colorPass = createColorPass(vulkan);
    createCommandPool();
    debugLogger->info("Engine Init Complete");
}

void Engine::prepare()
{
    createShadowFramebuffers();
    createColorFramebuffers();
    createSyncObjects();

    createCommandBuffers();
    vulkan->prepared = true;
    debugLogger->info("Engine Prepared");
}

Engine::~Engine()
{
    if (enableValidationLayers)
    {
        vulkan->instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldy);
    }
    vulkan->device.freeCommandBuffers(vulkan->commandPool, static_cast<uint32_t>(commandBuffers.size()),
                                      commandBuffers.data());
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (!renderFinishedSemaphores.empty() && renderFinishedSemaphores[i])
        {
            vulkan->device.destroySemaphore(renderFinishedSemaphores[i]);
        }
        if (!presentFinishedSemaphores.empty() && presentFinishedSemaphores[i])
        {
            vulkan->device.destroySemaphore(presentFinishedSemaphores[i]);
        }
        if (!waitFences.empty() && waitFences[i])
        {
            vulkan->device.destroyFence(waitFences[i]);
        }
    }
    debugLogger->info("Engine destroyed");
}

void Engine::renderShadows(vk::CommandBuffer commandBuffer, int32_t currentImage)
{
    vk::Viewport viewport{};
    viewport.width = vulkan->shadowSize;
    viewport.height = vulkan->shadowSize;
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor{};
    scissor.extent.width = vulkan->shadowSize;
    scissor.extent.height = vulkan->shadowSize;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    commandBuffer.setScissor(0, 1, &scissor);

    commandBuffer.setLineWidth(1.0F);

    std::array<vk::ClearValue, 2> clearValues = {};
    clearValues[0].color = std::array<float, 4>{0.F, 0.F, 0.F, 0.F};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0F, 0};

    vk::RenderPassBeginInfo sunPassInfo = {};
    sunPassInfo.renderPass = vulkan->shadowPass;
    sunPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    sunPassInfo.renderArea.extent.width = vulkan->shadowSize;
    sunPassInfo.renderArea.extent.height = vulkan->shadowSize;
    sunPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    sunPassInfo.pClearValues = clearValues.data();
    sunPassInfo.framebuffer = shadowFbs[currentImage].framebuffer;

    commandBuffer.beginRenderPass(sunPassInfo, vk::SubpassContents::eInline);
    scene->drawShadow(commandBuffer, currentImage);
    commandBuffer.endRenderPass();
}

void Engine::renderColors(vk::CommandBuffer commandBuffer, int32_t currentImage)
{
    vk::Viewport viewport{};
    viewport.width = static_cast<float>(vulkan->width);
    viewport.height = static_cast<float>(vulkan->height);
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor{};
    scissor.extent = vulkan->swapChainExtent;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    commandBuffer.setScissor(0, 1, &scissor);

    commandBuffer.setLineWidth(1.0F);

    std::array<vk::ClearValue, 2> clearValues = {};
    clearValues[0].color = std::array<float, 4>{0.0F, 0.0F, 0.0F, 0.0F};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0F, 0};

    vk::RenderPassBeginInfo colorPassInfo = {};
    colorPassInfo.renderPass = vulkan->colorPass;
    colorPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    colorPassInfo.renderArea.extent = vulkan->swapChainExtent;
    colorPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    colorPassInfo.pClearValues = clearValues.data();
    colorPassInfo.framebuffer = swapChainFbs[currentImage].framebuffer;

    commandBuffer.beginRenderPass(colorPassInfo, vk::SubpassContents::eInline);
    scene->drawColor(commandBuffer, currentImage);
    if (vulkan->showOverlay)
    {
        overlay->draw(commandBuffer, currentImage);
    }
    commandBuffer.endRenderPass();
}

void Engine::createCommandBuffers()
{
    commandBuffers.resize(swapChainFbs.size());

    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = vulkan->commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    commandBuffers = vulkan->device.allocateCommandBuffers(allocInfo);

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
    if (!vulkan->prepared)
    {
        return;
    }

    if (overlay->update)
    {
        overlay->cleanup();
        overlay->recreate();
        createCommandBuffers();
    }

    CheckResult(vulkan->device.waitForFences(1, &waitFences[vulkan->currentImage], VK_FALSE, UINT64_MAX));
    CheckResult(vulkan->device.resetFences(1, &waitFences[vulkan->currentImage]));

    uint32_t currentBuffer;
    auto result = vulkan->device.acquireNextImageKHR(
        vulkan->swapChain, UINT64_MAX, presentFinishedSemaphores[vulkan->currentImage], nullptr, &currentBuffer);

    if ((result == vk::Result::eErrorOutOfDateKHR) || (result == vk::Result::eSuboptimalKHR))
    {
        resizeWindow();
        // return;
    }
    else
    {
        CheckResult(result);
    }

    scene->update(currentBuffer, deltaTime);

    const vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo = {};
    submitInfo.pWaitDstStageMask = &waitStages;
    submitInfo.pWaitSemaphores = &presentFinishedSemaphores[vulkan->currentImage];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[vulkan->currentImage];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentBuffer];
    submitInfo.commandBufferCount = 1;
    vulkan->graphicsQueue.submit(1, &submitInfo, waitFences[vulkan->currentImage]);

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.pNext = nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vulkan->swapChain;
    presentInfo.pImageIndices = &currentBuffer;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[vulkan->currentImage];
    presentInfo.waitSemaphoreCount = 1;
    result = vulkan->presentQueue.presentKHR(&presentInfo);

    if (result == vk::Result::eErrorOutOfDateKHR)
    {
        resizeWindow();
        return;
    }
    if (result != vk::Result::eSuccess)
    {
        CheckResult(result);
    }
    if (vulkan->updateCommandBuffer)
    {
        updateWindow();
        vulkan->updateCommandBuffer = false;
    }

    vulkan->currentImage = (vulkan->currentImage + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Engine::updateWindow()
{
    if (!vulkan->prepared)
    {
        return;
    }
    vulkan->prepared = false;

    vulkan->device.waitIdle();

    for (auto &framebuffer : swapChainFbs)
    {
        framebuffer.cleanup();
    }

    vulkan->device.destroyRenderPass(vulkan->colorPass);

    for (auto imageView : vulkan->swapChainImageViews)
    {
        vulkan->device.destroyImageView(imageView);
    }

    vulkan->device.destroySwapchainKHR(vulkan->swapChain);

    vulkan->device.freeCommandBuffers(vulkan->commandPool, static_cast<uint32_t>(commandBuffers.size()),
                                      commandBuffers.data());

    createSwapChain();
    vulkan->colorPass = createColorPass(vulkan);
    createColorFramebuffers();
    createCommandBuffers();
    vulkan->device.waitIdle();

    vulkan->prepared = true;
}

void Engine::resizeWindow()
{
    if (!vulkan->prepared)
    {
        return;
    }
    vulkan->prepared = false;

    int width = 0;
    int height = 0;
    while (width == 0 || height == 0)
    {
        std::tie(width, height) = vulkan->window->getFrameBufferSize();
        vulkan->window->wait();
    }
    vulkan->device.waitIdle();
    vulkan->width = width;
    vulkan->height = height;

    for (auto &framebuffer : swapChainFbs)
    {
        framebuffer.cleanup();
    }

    vulkan->device.destroyRenderPass(vulkan->colorPass);

    for (auto imageView : vulkan->swapChainImageViews)
    {
        vulkan->device.destroyImageView(imageView);
    }

    vulkan->device.destroySwapchainKHR(vulkan->swapChain);

    vulkan->device.freeCommandBuffers(vulkan->commandPool, static_cast<uint32_t>(commandBuffers.size()),
                                      commandBuffers.data());

    scene->cleanup();
    overlay->cleanup();

    createSwapChain();
    vulkan->colorPass = createColorPass(vulkan);

    scene->recreate();
    overlay->recreate();

    createColorFramebuffers();

    createCommandBuffers();
    vulkan->device.waitIdle();

    vulkan->prepared = true;
    debugLogger->info("Resized window to {}x{}", width, height);
}

void Engine::createInstance()
{

    vk::ApplicationInfo appInfo = {};
    appInfo.pApplicationName = vulkan->name.c_str();
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

    vulkan->instance = vk::createInstance(createInfo);
    debugLogger->info("Created Vulkan instance");
}

void Engine::pickPhysicalDevice()
{
    auto devices = vulkan->instance.enumeratePhysicalDevices();

    assert(!devices.empty());

    for (const auto &device : devices)
    {
        if (isDeviceSuitable(device))
        {
            vulkan->properties = device.getProperties();
            debugLogger->info("Physical Device: {}", vulkan->properties.deviceName);
            vulkan->physicalDevice = device;
            vulkan->msaaSamples = getMaxUsableSampleCount();
            return;
        }
    }

    assert(vulkan->physicalDevice);
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

        auto presentSupport = device.getSurfaceSupportKHR(i, vulkan->surface);

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
    details.capabilities = device.getSurfaceCapabilitiesKHR(vulkan->surface);
    details.formats = device.getSurfaceFormatsKHR(vulkan->surface);
    details.presentModes = device.getSurfacePresentModesKHR(vulkan->surface);
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
    QueueFamilyIndices indices = findQueueFamiles(vulkan->physicalDevice);

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

        dldy.init(vulkan->instance);

        vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
        debugInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        debugInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        debugInfo.pfnUserCallback = debugCallback;

        debugMessenger = vulkan->instance.createDebugUtilsMessengerEXT(debugInfo, nullptr, dldy);
        vulkan->device = vulkan->physicalDevice.createDevice(createInfo, nullptr, dldy);
    }
    else
    {
        vulkan->device = vulkan->physicalDevice.createDevice(createInfo);
    }

    vulkan->graphicsQueue = vulkan->device.getQueue(indices.graphicsFamily.value(), 0);
    vulkan->presentQueue = vulkan->device.getQueue(indices.presentFamily.value(), 0);
    debugLogger->info("Created Logical Device");
}

void Engine::createAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = vulkan->physicalDevice;
    allocatorInfo.device = vulkan->device;

    CheckResult(vmaCreateAllocator(&allocatorInfo, &vulkan->allocator));
    debugLogger->info("Created Allocator");
}

void Engine::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vulkan->physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities, vulkan->width, vulkan->height);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.surface = vulkan->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    QueueFamilyIndices indices = findQueueFamiles(vulkan->physicalDevice);
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

    vulkan->swapChain = vulkan->device.createSwapchainKHR(createInfo);

    vulkan->swapChainImages = vulkan->device.getSwapchainImagesKHR(vulkan->swapChain);
    vulkan->swapChainImageFormat = surfaceFormat.format;
    vulkan->swapChainExtent = extent;
    vulkan->swapChainImageViews.resize(vulkan->swapChainImages.size());

    for (size_t i = 0; i < vulkan->swapChainImages.size(); i++)
    {
        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = vulkan->swapChainImages[i];
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = vulkan->swapChainImageFormat;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        vulkan->swapChainImageViews[i] = vulkan->device.createImageView(viewInfo);
    }
    debugLogger->info("Created SwapChain");
}

void Engine::createShadowFramebuffers()
{
    shadowDepth.vulkan = vulkan;
    shadowDepth.format = vulkan->findDepthFormat();
    shadowDepth.imageUsage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc;
    shadowDepth.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    shadowDepth.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    shadowDepth.aspect = vk::ImageAspectFlagBits::eDepth;
    shadowDepth.resize(static_cast<int>(vulkan->shadowSize), static_cast<int>(vulkan->shadowSize));

    shadowFbs.resize(vulkan->swapChainImageViews.size());
    for (size_t i = 0; i < vulkan->swapChainImageViews.size(); i++)
    {
        shadowFbs[i].vulkan = vulkan;
        shadowFbs[i].renderPass = vulkan->shadowPass;
        shadowFbs[i].width = vulkan->shadowSize;
        shadowFbs[i].height = vulkan->shadowSize;
        shadowFbs[i].attachments = {scene->shadow.imageView, shadowDepth.imageView};
        shadowFbs[i].create();
    }
    debugLogger->info("Created Framebuffer for shadows");
}

void Engine::createColorFramebuffers()
{
    colorAttachment.vulkan = vulkan;
    colorAttachment.format = vulkan->swapChainImageFormat;
    colorAttachment.numSamples = vulkan->msaaSamples;
    colorAttachment.imageUsage =
        vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
    colorAttachment.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    colorAttachment.layout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachment.resize(vulkan->swapChainExtent.width, vulkan->swapChainExtent.height);

    depthAttachment.vulkan = vulkan;
    depthAttachment.format = vulkan->findDepthFormat();
    depthAttachment.numSamples = vulkan->msaaSamples;
    depthAttachment.imageUsage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc;
    depthAttachment.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    depthAttachment.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    depthAttachment.aspect = vk::ImageAspectFlagBits::eDepth;
    depthAttachment.resize(vulkan->swapChainExtent.width, vulkan->swapChainExtent.height);

    swapChainFbs.resize(vulkan->swapChainImageViews.size());

    for (size_t i = 0; i < vulkan->swapChainImageViews.size(); i++)
    {
        swapChainFbs[i].vulkan = vulkan;
        swapChainFbs[i].renderPass = vulkan->colorPass;
        swapChainFbs[i].width = vulkan->swapChainExtent.width;
        swapChainFbs[i].height = vulkan->swapChainExtent.height;
        swapChainFbs[i].attachments = {colorAttachment.imageView, depthAttachment.imageView,
                                       vulkan->swapChainImageViews[i]};
        swapChainFbs[i].create();
    }
    debugLogger->info("Created Framebuffer for display");
}

void Engine::createCommandPool()
{
    QueueFamilyIndices QueueFamilyIndices = findQueueFamiles(vulkan->physicalDevice);

    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.queueFamilyIndex = QueueFamilyIndices.graphicsFamily.value();

    vulkan->commandPool = vulkan->device.createCommandPool(poolInfo);
    debugLogger->info("Created Command Pool");
}

void Engine::createSyncObjects()
{
    presentFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    waitFences.resize(MAX_FRAMES_IN_FLIGHT);

    for (auto &semaphore : presentFinishedSemaphores)
    {
        semaphore = vulkan->device.createSemaphore({});
    }
    for (auto &semaphore : renderFinishedSemaphores)
    {
        semaphore = vulkan->device.createSemaphore({});
    }
    for (auto &fence : waitFences)
    {
        vk::FenceCreateInfo fenceInfo = {};
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        fence = vulkan->device.createFence(fenceInfo);
    }
    debugLogger->info("Created Sync Objects");
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
    const auto &colorCounts = vulkan->properties.limits.framebufferColorSampleCounts;
    const auto &depthCounts = vulkan->properties.limits.framebufferDepthSampleCounts;

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
        if (availablePresentMode == vulkan->defaultPresentMode)
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

} // namespace tat