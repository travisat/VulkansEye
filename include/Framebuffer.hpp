#pragma once

#include <vulkan/vulkan.hpp>
#include <spdlog/spdlog.h>

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
    std::shared_ptr<spdlog::logger> debugLogger;
    void createRenderPass();
};

} // namespace tat