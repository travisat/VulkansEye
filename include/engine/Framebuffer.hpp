#pragma once
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{

class Framebuffer
{
  public:
    Framebuffer() = default;
    ~Framebuffer();

    void create();

    vk::RenderPass renderPass;
    int32_t width = 0;
    int32_t height = 0;
    int32_t layers = 1;

    vk::Framebuffer framebuffer;
    std::vector<vk::ImageView> attachments;

  private:
    void createRenderPass();
};

} // namespace tat