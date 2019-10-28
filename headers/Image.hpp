#pragma once

#include <gli/gli.hpp>
#include <stb_image.h>

#include "Buffer.hpp"
#include "Vulkan.hpp"
#include "vulkan/vulkan_core.h"

namespace tat
{

class Image
{
  public:
    Vulkan *vulkan = nullptr;
    VkImage image = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    VmaAllocation allocation{};

    VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT;
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_UNKNOWN;
    VkImageCreateFlags flags = 0;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;

    VkFilter magFilter = VK_FILTER_LINEAR;
    VkFilter minFilter = VK_FILTER_LINEAR;
    VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkBool32 anistropyEnable = VK_TRUE;
    float maxAnisotropy = 16.F;
    VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    VkBool32 unnormalizedCoordinates = VK_FALSE;
    VkBool32 compareEnable = VK_FALSE;
    VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
    VkSamplerMipmapMode mipmapLod = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkDeviceSize size = 0;
    VkImageView imageView = VK_NULL_HANDLE;

    int width = 0;
    int height = 0;
    int channels = 4; // rgba
    int layers = 1;
    uint32_t mipLevels = 1;

    ~Image();

    void loadSTB(std::string path); // use stb_image.h to load most normal image formats
    void loadTextureCube(std::string path);

    void createSampler();

    void copyFrom(VkCommandBuffer commandBuffer, const Buffer &buffer);
    void copyFrom(const Buffer &buffer);
    void resize(int width, int height);

    void generateMipmaps();
    void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
    void transitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

  private:
    // create VkImage allocation
    void allocate();
    // destroy VkImage and VkImageView if they exist
    void deallocate();
    void createImageView();

    static auto hasStencilComponent(VkFormat format) -> bool;
};

} // namespace tat