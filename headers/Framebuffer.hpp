#pragma once
#include "Vulkan.hpp"
#include "Image.hpp"

namespace tat
{

class Framebuffer
{
public:
    Vulkan *vulkan;
    VkRenderPass renderPass;
    int32_t width;
    int32_t height;

    VkFramebuffer framebuffer;
    std::vector<VkImageView> attachments;

    ~Framebuffer();
    void create();
    void cleanup();

private:
    void createRenderPass();

};

} // namespace tat