#pragma once
#include "Vulkan.hpp"
#include "Image.hpp"
#include <memory>

namespace tat
{

class Framebuffer
{
public:
    std::shared_ptr<Vulkan> vulkan;
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