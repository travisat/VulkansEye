#pragma once
#include "Vulkan.hpp"
#include "Image.hpp"

namespace tat
{

class Framebuffer
{
public:
    Vulkan *vulkan;
    vk::RenderPass renderPass;
    int32_t width;
    int32_t height;
    int32_t layers = 1;

    vk::Framebuffer framebuffer;
    std::vector<vk::ImageView> attachments;

    ~Framebuffer();
    void create();
    void cleanup();

private:
    void createRenderPass();

};

} // namespace tat