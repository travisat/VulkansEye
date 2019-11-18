#pragma once
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{

class RenderPass
{
  public:
    vk::RenderPass renderPass;
    void create();
    void recreate();
    void destroy();
    void loadColor();
    void loadShadow();

    vk::RenderPassCreateInfo renderPassInfo {};
    vk::SubpassDescription subpass {};
    std::vector<vk::AttachmentDescription> attachments{};
    vk::AttachmentReference colorReference {};
    vk::AttachmentReference depthReference {};
    vk::AttachmentReference resolveReference {};
    std::vector<vk::SubpassDependency> dependencies{};
};

} // namespace tat