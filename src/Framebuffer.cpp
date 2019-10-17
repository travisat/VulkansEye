#include "Framebuffer.hpp"
#include "helpers.h"

namespace tat
{

Framebuffer::~Framebuffer()
{
    cleanup();
}

void Framebuffer::create()
{
    VkFormat depthFormat = vulkan->findDepthFormat();
    depthAttachment.vulkan = vulkan;
    depthAttachment.format = depthFormat;
    depthAttachment.numSamples = vulkan->msaaSamples;
    depthAttachment.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    depthAttachment.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthAttachment.resize(vulkan->swapChainExtent.width, vulkan->swapChainExtent.height);
    attachments.insert(attachments.begin(), depthAttachment.imageView);

    VkFormat colorFormat = vulkan->swapChainImageFormat;
    colorAttachment.vulkan = vulkan;
    colorAttachment.format = colorFormat;
    colorAttachment.numSamples = vulkan->msaaSamples;
    colorAttachment.imageUsage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    colorAttachment.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    colorAttachment.layout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resize(vulkan->swapChainExtent.width, vulkan->swapChainExtent.height);
    attachments.insert(attachments.begin(), colorAttachment.imageView);

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vulkan->renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;
    CheckResult(vkCreateFramebuffer(vulkan->device, &framebufferInfo, nullptr, &framebuffer));
}

void Framebuffer::cleanup()
{
    vkDestroyFramebuffer(vulkan->device, framebuffer, nullptr);
}

} // namespace tat