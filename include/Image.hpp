#pragma once

#include "Buffer.hpp"

namespace tat
{

class Image
{
  public:
    vk::Image image = nullptr;
    vk::ImageView imageView = nullptr;
    vk::Sampler sampler = nullptr;

    vk::ImageCreateInfo imageInfo {};
    vk::SamplerCreateInfo samplerInfo {};
    vk::ImageViewCreateInfo imageViewInfo {};
    vk::ImageLayout currentLayout = vk::ImageLayout::eUndefined;

    // default createinfo settings for image/imageview/sampler
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_UNKNOWN;

    Image();
    ~Image();

    //load info into image
    void load(const std::string &path); // use gli to load dds/ktx supports cubemaps

    void createSampler();
    
    void resize(int width, int height);

    void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void transitionImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
    VmaAllocation allocation{};
    std::string path;
    // create VkImage allocation
    void allocate();
    // destroy VkImage and VkImageView if they exist
    void deallocate();
    void createImageView();
};

} // namespace tat