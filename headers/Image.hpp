#pragma once

#include "State.hpp"
#include "Buffer.hpp"

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
        VkImageCreateFlags flags, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers);

  ~Image();

  void createImageView(VkImageViewType viewType, VkImageAspectFlags aspectFlags);
  void createImageView(VkImageViewType viewType, VkImageAspectFlags aspectFlags, uint32_t layerCount);

  void copy(Buffer *buffer);
  void copy(Buffer *buffer, uint32_t layerCount);

  void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
  void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount);

  void generateMipmaps();

  bool hasStencilComponent(VkFormat format);

  int width = 0;
  int height = 0;
  int channels = 0;
  int layers = 0;
  uint32_t mipLevels = 0;

  VkImage image;
  VmaAllocation allocation;
  VkFormat format;
  VkDeviceSize size;
  VkImageView imageView;

private:
  State *state;
};