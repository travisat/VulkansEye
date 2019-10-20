#include "Engine.hpp"
#include "helpers.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                    void *pUserData)
{
    tat::Trace("Validation: ", pCallbackData->pMessage);
    return VK_FALSE;
};

namespace tat
{

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
};

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
};

void Engine::init()
{

    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createAllocator();

    createSwapChain();
    createRenderPass(vulkan->renderPass, vulkan);
    createCommandPool();
    createFramebuffers();
    createSyncObjects();

    scene->create();
    overlay->create();

    createCommandBuffers();
    vulkan->prepared = true;
}

Engine::~Engine()
{
    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(vulkan->instance, debugMessenger, nullptr);
    }
    vkFreeCommandBuffers(vulkan->device, vulkan->commandPool, static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(vulkan->device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(vulkan->device, presentFinishedSemaphores[i], nullptr);
        vkDestroyFence(vulkan->device, waitFences[i], nullptr);
    }
}

void Engine::createCommandBuffers()
{
    commandBuffers.resize(swapChainFbs.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vulkan->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    CheckResult(vkAllocateCommandBuffers(vulkan->device, &allocInfo, commandBuffers.data()));
    recordCommandBuffers();
}

void Engine::recordCommandBuffers()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 0.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vulkan->renderPass;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = vulkan->swapChainExtent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    for (uint32_t i = 0; i < commandBuffers.size(); ++i)
    {
        renderPassInfo.framebuffer = swapChainFbs[i].framebuffer;

        VkCommandBuffer commandBuffer = commandBuffers[i];

        CheckResult(vkBeginCommandBuffer(commandBuffer, &beginInfo));
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.width = (float)vulkan->width;
        viewport.height = (float)vulkan->height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.extent = vulkan->swapChainExtent;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdSetLineWidth(commandBuffer, 1.0f);

        scene->draw(commandBuffer, i);

        if (vulkan->showOverlay)
        {
            overlay->draw(commandBuffer, i);
        }

        vkCmdEndRenderPass(commandBuffer);
        CheckResult(vkEndCommandBuffer(commandBuffer));
    }
}

void Engine::drawFrame()
{
    if (!vulkan->prepared)
        return;

    if (overlay->update)
    {
        overlay->cleanup();
        overlay->recreate();
        createCommandBuffers();
    }

    CheckResult(vkWaitForFences(vulkan->device, 1, &waitFences[vulkan->currentImage], VK_FALSE, UINT64_MAX));
    CheckResult(vkResetFences(vulkan->device, 1, &waitFences[vulkan->currentImage]));

    uint32_t currentBuffer; // three buffers per swapchain only two images per max
                            // frames in flight idk
    VkResult result =
        vkAcquireNextImageKHR(vulkan->device, vulkan->swapChain, UINT64_MAX,
                              presentFinishedSemaphores[vulkan->currentImage], VK_NULL_HANDLE, &currentBuffer);

    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
    {
        resizeWindow();
        // return;
    }
    else
    {
        CheckResult(result);
    }

    scene->update(currentBuffer);

    const VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &waitStages;
    submitInfo.pWaitSemaphores = &presentFinishedSemaphores[vulkan->currentImage];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[vulkan->currentImage];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentBuffer];
    submitInfo.commandBufferCount = 1;
    CheckResult(vkQueueSubmit(vulkan->graphicsQueue, 1, &submitInfo, waitFences[vulkan->currentImage]));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vulkan->swapChain;
    presentInfo.pImageIndices = &currentBuffer;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[vulkan->currentImage];
    presentInfo.waitSemaphoreCount = 1;
    result = vkQueuePresentKHR(vulkan->presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        resizeWindow();
        return;
    }
    else if (result != VK_SUCCESS)
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

    vkDeviceWaitIdle(vulkan->device);

    for (auto &framebuffer : swapChainFbs)
    {
        framebuffer.cleanup();
    }

    vkDestroyRenderPass(vulkan->device, vulkan->renderPass, nullptr);

    for (auto imageView : vulkan->swapChainImageViews)
    {
        vkDestroyImageView(vulkan->device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(vulkan->device, vulkan->swapChain, nullptr);

    vkFreeCommandBuffers(vulkan->device, vulkan->commandPool, static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());

    createSwapChain();
    createRenderPass(vulkan->renderPass, vulkan);
    createFramebuffers();
    createCommandBuffers();
    vkDeviceWaitIdle(vulkan->device);

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
        glfwGetFramebufferSize(vulkan->window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(vulkan->device);
    vulkan->width = width;
    vulkan->height = height;

    for (auto &framebuffer : swapChainFbs)
    {
        framebuffer.cleanup();
    }

    vkDestroyRenderPass(vulkan->device, vulkan->renderPass, nullptr);

    for (auto imageView : vulkan->swapChainImageViews)
    {
        vkDestroyImageView(vulkan->device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(vulkan->device, vulkan->swapChain, nullptr);

    vkFreeCommandBuffers(vulkan->device, vulkan->commandPool, static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());

    scene->cleanup();
    overlay->cleanup();

    createSwapChain();
    createRenderPass(vulkan->renderPass, vulkan);
    createFramebuffers();

    scene->recreate();
    overlay->recreate();

    createCommandBuffers();
    vkDeviceWaitIdle(vulkan->device);

    vulkan->prepared = true;
    Trace("Resized window to ", width, "x", height, " at ", Timer::systemTime());
}

void Engine::createInstance()
{

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkans Eye";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    CheckResult(vkCreateInstance(&createInfo, nullptr, &vulkan->instance));
}

void Engine::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void Engine::setupDebugMessenger()
{
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    CheckResult(CreateDebugUtilsMessengerEXT(vulkan->instance, &createInfo, nullptr, &debugMessenger));
}

void Engine::createSurface()
{
    CheckResult(glfwCreateWindowSurface(vulkan->instance, vulkan->window, nullptr, &vulkan->surface));
}

void Engine::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkan->instance, &deviceCount, nullptr);

    assert(deviceCount != 0);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vulkan->instance, &deviceCount, devices.data());

    for (const auto &device : devices)
    {
        if (isDeviceSuitable(device))
        {
            vkGetPhysicalDeviceProperties(device, &vulkan->properties);
            Trace("Physical Device: ", vulkan->properties.deviceName);
            vulkan->physicalDevice = device;
            vulkan->msaaSamples = getMaxUsableSampleCount();
            break;
        }
    }

    assert(vulkan->physicalDevice != VK_NULL_HANDLE);
}

bool Engine::isDeviceSuitable(VkPhysicalDevice const &device)
{
    QueueFamilyIndices indicies = findQueueFamiles(device);

    bool extensionsSupported = checkDeviceExtensionsSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indicies.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy &&
           supportedFeatures.tessellationShader;
};

QueueFamilyIndices Engine::findQueueFamiles(VkPhysicalDevice const &device)
{
    QueueFamilyIndices indices;

    uint32_t QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &QueueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &QueueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vulkan->surface, &presentSupport);

        if (queueFamily.queueCount > 0 && presentSupport)
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

SwapChainSupportDetails Engine::querySwapChainSupport(VkPhysicalDevice const &device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkan->surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan->surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan->surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan->surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan->surface, &presentModeCount,
                                                  details.presentModes.data());
    }

    return details;
}

bool Engine::checkDeviceExtensionsSupport(VkPhysicalDevice const &device)
{
    uint32_t extensionsCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void Engine::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamiles(vulkan->physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;
    deviceFeatures.tessellationShader = VK_TRUE;
    deviceFeatures.geometryShader = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.enabledLayerCount = 0;

    CheckResult(vkCreateDevice(vulkan->physicalDevice, &createInfo, nullptr, &vulkan->device));

    vkGetDeviceQueue(vulkan->device, indices.graphicsFamily.value(), 0, &vulkan->graphicsQueue);
    vkGetDeviceQueue(vulkan->device, indices.presentFamily.value(), 0, &vulkan->presentQueue);
}

void Engine::createAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = vulkan->physicalDevice;
    allocatorInfo.device = vulkan->device;

    CheckResult(vmaCreateAllocator(&allocatorInfo, &vulkan->allocator));
}

void Engine::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vulkan->physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, vulkan->width, vulkan->height);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vulkan->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamiles(vulkan->physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    CheckResult(vkCreateSwapchainKHR(vulkan->device, &createInfo, nullptr, &vulkan->swapChain));

    vkGetSwapchainImagesKHR(vulkan->device, vulkan->swapChain, &imageCount, nullptr);
    vulkan->swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(vulkan->device, vulkan->swapChain, &imageCount, vulkan->swapChainImages.data());

    vulkan->swapChainImageFormat = surfaceFormat.format;
    vulkan->swapChainExtent = extent;

    vulkan->swapChainImageViews.resize(vulkan->swapChainImages.size());

    for (size_t i = 0; i < vulkan->swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = vulkan->swapChainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = vulkan->swapChainImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        CheckResult(vkCreateImageView(vulkan->device, &viewInfo, nullptr, &vulkan->swapChainImageViews[i]));
    }
}

void Engine::createFramebuffers()
{
    VkFormat colorFormat = vulkan->swapChainImageFormat;
    colorAttachment.vulkan = vulkan;
    colorAttachment.format = colorFormat;
    colorAttachment.numSamples = vulkan->msaaSamples;
    colorAttachment.imageUsage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    colorAttachment.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resize(vulkan->swapChainExtent.width, vulkan->swapChainExtent.height);

    VkFormat depthFormat = vulkan->findDepthFormat();
    depthAttachment.vulkan = vulkan;
    depthAttachment.format = depthFormat;
    depthAttachment.numSamples = vulkan->msaaSamples;
    depthAttachment.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    depthAttachment.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthAttachment.resize(vulkan->swapChainExtent.width, vulkan->swapChainExtent.height);

    swapChainFbs.resize(vulkan->swapChainImageViews.size());

    for (size_t i = 0; i < vulkan->swapChainImageViews.size(); i++)
    {
        swapChainFbs[i].vulkan = vulkan;
        swapChainFbs[i].renderPass = vulkan->renderPass;
        swapChainFbs[i].width = vulkan->swapChainExtent.width;
        swapChainFbs[i].height = vulkan->swapChainExtent.height;
        swapChainFbs[i].attachments = {colorAttachment.imageView, depthAttachment.imageView,
                                       vulkan->swapChainImageViews[i]};
        swapChainFbs[i].create();
    }
}

void Engine::createCommandPool()
{
    QueueFamilyIndices QueueFamilyIndices = findQueueFamiles(vulkan->physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = QueueFamilyIndices.graphicsFamily.value();

    CheckResult(vkCreateCommandPool(vulkan->device, &poolInfo, nullptr, &vulkan->commandPool));
}

void Engine::createSyncObjects()
{
    presentFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    waitFences.resize(MAX_FRAMES_IN_FLIGHT);

    for (auto &semaphore : presentFinishedSemaphores)
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        CheckResult(vkCreateSemaphore(vulkan->device, &semaphoreInfo, nullptr, &semaphore));
    }
    for (auto &semaphore : renderFinishedSemaphores)
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        CheckResult(vkCreateSemaphore(vulkan->device, &semaphoreInfo, nullptr, &semaphore));
    }
    for (auto &fence : waitFences)
    {
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        CheckResult(vkCreateFence(vulkan->device, &fenceInfo, nullptr, &fence));
    }
}

std::vector<const char *> Engine::getRequiredExtensions()
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

VkSampleCountFlagBits Engine::getMaxUsableSampleCount()
{
    VkSampleCountFlags counts = std::min(vulkan->properties.limits.framebufferColorSampleCounts,
                                         vulkan->properties.limits.framebufferDepthSampleCounts);
    if (counts & VK_SAMPLE_COUNT_64_BIT)
    {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT)
    {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT)
    {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT)
    {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT)
    {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT)
    {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

VkSurfaceFormatKHR Engine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Engine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vulkan->defaultPresentMode)
        {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Engine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, uint32_t windowWidth,
                                    uint32_t windowHeight)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actualExtent = {windowWidth, windowHeight};

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

} // namespace tat