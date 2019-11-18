#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <vk_mem_alloc.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"

namespace tat
{

class Image
{
  public:
    // don't use uniqueimage, vma is used for memory for it
    vk::Image image;
    vk::ImageView imageView;
    vk::Sampler sampler;

    vk::ImageCreateInfo imageInfo{};
    vk::SamplerCreateInfo samplerInfo{};
    vk::ImageViewCreateInfo imageViewInfo{};
    vk::ImageLayout currentLayout = vk::ImageLayout::eUndefined;

    // default createinfo settings for image/imageview/sampler
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_UNKNOWN;

    Image();
    ~Image() = default;

    void create();
    void destroy();

    // load info into image
    void load(const std::string &path); // use gli to load dds/ktx supports cubemaps

    void createSampler();

    void resize(int width, int height);
    void resize(vk::Extent2D extent)
    {
        resize(extent.width, extent.height);
    }

    void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void transitionImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

  private:
    int32_t allocId{};
    std::string path;

    void createImageView();
};

} // namespace tat