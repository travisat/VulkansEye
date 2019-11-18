#include "engine/Framebuffer.hpp"
#include "State.hpp"

namespace tat
{

void Framebuffer::create()
{
    auto& engine = State::instance().engine;
    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = layers;
    framebuffer = engine.device.createFramebuffer(framebufferInfo);
}

void Framebuffer::destroy()
{
    auto& engine = State::instance().engine;
    engine.device.destroyFramebuffer(framebuffer);
}

} // namespace tat