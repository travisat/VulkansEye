#pragma once

#include <gli/gli.hpp>
#include <stb_image.h>

#include "Vulkan.hpp"
#include "Buffer.hpp"



class Image
{
public:
      tat::Vulkan *vulkan = nullptr;
      VkImage image = VK_NULL_HANDLE;
      VmaAllocation allocation{};

      VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
      VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
      VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT;
      VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
      VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_UNKNOWN;
      VkImageCreateFlags flags = 0;
      VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

      VkDeviceSize size = 0;
      VkImageView imageView = VK_NULL_HANDLE;

      ImageType type = ImageType::unknown;
      int width = 0;
      int height = 0;
      int channels = 4; //rgba
      int layers = 1;
      uint32_t mipLevels = 1;

      std::string name = "Unknown";

      ~Image();

        //create VkImage allocation
      VkResult allocate();

      VkResult loadFile(std::string path);
      VkResult loadTextureCube(std::string path);

      VkResult copyFrom(const Buffer &buffer, uint32_t layerCount = 1);
      VkResult resize(int width, int height, int channels = 4, int layers = 1);

      VkResult createImageView(VkImageViewType viewType, VkImageAspectFlags aspectFlags, uint32_t layerCount = 1);

     VkResult transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount = 1);

      VkResult generateMipmaps();

private:
    
      //destroy VkImage and VkImageView if they exist
      void deallocate();

      VkResult loadSTB(std::string path); //use stb_image.h to load most normal image formats
      VkResult loadGLI(std::string path); //use gli to load texture (dds/ktx/kmg) files)

      bool hasStencilComponent(VkFormat format);
};