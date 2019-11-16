#pragma once

#include "Buffer.hpp"

namespace tat
{

class Image
{
  public:
    //don't use uniqueimage, vma is used for memory for it
    vk::Image image;
    vk::UniqueImageView imageView;
    vk::UniqueSampler sampler;

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

    void allocate();
    // destroy VkImage and VkImageView if they exist
    void deallocate();

  private:
    VmaAllocation allocation{};
    bool allocated = false;
    std::string path;
    
    void createImageView();
};

} // namespace tat