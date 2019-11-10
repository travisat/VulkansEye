#include "Framebuffer.hpp"

namespace tat
{

Framebuffer::~Framebuffer()
{
    cleanup();
}

void Framebuffer::create()
{
    debugLogger = spdlog::get("debugLogger");
    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = layers;
    framebuffer = vulkan->device.createFramebuffer(framebufferInfo);
}

void Framebuffer::cleanup()
{
    if (framebuffer)
    {
        vulkan->device.destroyFramebuffer(framebuffer);
    }
}

} // namespace tat