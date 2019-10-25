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

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = layers;
    CheckResult(vkCreateFramebuffer(vulkan->device, &framebufferInfo, nullptr, &framebuffer));
}

void Framebuffer::cleanup()
{
    if (framebuffer != nullptr)
    {
        vkDestroyFramebuffer(vulkan->device, framebuffer, nullptr);
    }
}

} // namespace tat