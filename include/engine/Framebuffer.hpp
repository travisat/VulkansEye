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
    vk::RenderPass renderPass;
    int32_t width;
    int32_t height;
    int32_t layers = 1;

    vk::UniqueFramebuffer framebuffer;
    std::vector<vk::ImageView> attachments;

    void create();
    void recreate();

  private:
    void createRenderPass();
};

} // namespace tat