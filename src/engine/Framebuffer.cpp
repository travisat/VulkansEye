#include "engine/Framebuffer.hpp"
#include "State.hpp"

namespace tat
{

Framebuffer::~Framebuffer()
{
    auto &device = State::instance().engine.device;
    device.destroy(framebuffer);
}

void Framebuffer::create()
{
    auto &engine = State::instance().engine;
    vk::FramebufferCreateInfo framebufferInfo{};
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = layers;
    framebuffer = engine.device.create(framebufferInfo);
}

} // namespace tat