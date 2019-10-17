#pragma once
#include "Vulkan.hpp"
#include "Image.hpp"

namespace tat
{

class Framebuffer
{
public:
    Vulkan *vulkan;
    int32_t width;
    int32_t height;
    Image colorAttachment;
    Image depthAttachment;

    VkFramebuffer framebuffer;
    std::vector<VkImageView> attachments;

    ~Framebuffer();
    void create();
    void cleanup();

private:
    void createRenderPass();

};

} // namespace tat