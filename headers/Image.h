#pragma once

#include "Helpers.h"
#include "State.h"
#include "Buffer.h"

class Image
{
public:
    Image(State *state, VkFormat format,
          VkImageTiling tiling, VkSampleCountFlagBits numSamples,
          VkImageUsageFlags usage, VmaMemoryUsage memUsage,
          VkImageCreateFlags flags, std::string path);

    Image(State *state, VkFormat format,
          VkImageTiling tiling, VkSampleCountFlagBits numSamples,
          VkImageUsageFlags usage, VmaMemoryUsage memUsage,
          VkImageCreateFlags flags, uint32_t width, uint32_t height);

    Image(State *state, VkFormat format,
                 VkImageTiling tiling, VkSampleCountFlagBits numSamples,
                 VkImageUsageFlags usage, VmaMemoryUsage memUsage,
                 VkImageCreateFlags flags, uint32_t width, uint32_t height, uint32_t mipLevels);

    Image(State *state, VkFormat format,
      VkImageTiling tiling, VkSampleCountFlagBits numSamples,
      VkImageUsageFlags usage, VmaMemoryUsage memUsage,
      VkImageCreateFlags flags, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers);

    ~Image();

    VkImageView createImageView(VkImageViewType viewType, VkImageAspectFlags aspectFlags);
    VkImageView createImageView(VkImageViewType viewType, VkImageAspectFlags aspectFlags, uint32_t layerCount);
    void copy(Buffer *buffer);
    void copy(Buffer *buffer, uint32_t layerCount);
    void generateMipmaps();
    void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
    void Image::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount);
    bool hasStencilComponent(VkFormat format);

    VkImage getImage() { return image; };
    VmaAllocation getAllocation() { return allocation; };
    VkFormat getFormat() { return format; };
    VkDeviceSize getSize() { return size; };
    int getWidth() { return width; };
    int getHeight() { return height; };
    int getChannels() { return channels; };
    uint32_t getMipLevels() { return mipLevels; };
    void setMiplevels(uint32_t m) { mipLevels = m; };

private:
    State *state;
    VkImage image;
    VmaAllocation allocation;
    VkFormat format;
    VkDeviceSize size;

    int width;
    int height;
    int channels;
    uint32_t mipLevels;
};