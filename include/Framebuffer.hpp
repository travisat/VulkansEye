#pragma once

#include <vulkan/vulkan.hpp>

namespace tat
{

class Framebuffer
{
  public:
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