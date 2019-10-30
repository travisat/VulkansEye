#pragma once

#include <gli/gli.hpp>
#include <stb_image.h>

#include "Buffer.hpp"
#include "Vulkan.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"

namespace tat
{

class Image
{
  public:
    Vulkan *vulkan = nullptr;
    vk::Image image = nullptr;
    vk::Sampler sampler = nullptr;
    VmaAllocation allocation{};

    vk::Format format = vk::Format::eR8G8B8A8Unorm;
    vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
    vk::SampleCountFlagBits numSamples = vk::SampleCountFlagBits::e1;
    vk::ImageUsageFlags imageUsage = vk::ImageUsageFlagBits::eTransferSrc;
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_UNKNOWN;
    vk::ImageCreateFlags flags{};
    vk::ImageLayout layout = vk::ImageLayout::eUndefined;
    vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;
    vk::ImageViewType viewType = vk::ImageViewType::e2D;

    vk::Filter magFilter = vk::Filter::eLinear;
    vk::Filter minFilter = vk::Filter::eLinear;
    vk::SamplerAddressMode addressModeU = vk::SamplerAddressMode::eRepeat;
    vk::SamplerAddressMode addressModeV = vk::SamplerAddressMode::eRepeat;
    vk::SamplerAddressMode addressModeW = vk::SamplerAddressMode::eRepeat;
    vk::Bool32 anistropyEnable = VK_TRUE;
    float maxAnisotropy = 16.F;
    vk::BorderColor borderColor = vk::BorderColor::eIntOpaqueBlack;
    vk::Bool32 unnormalizedCoordinates = VK_FALSE;
    vk::Bool32 compareEnable = VK_FALSE;
    vk::CompareOp compareOp = vk::CompareOp::eAlways;
    vk::SamplerMipmapMode mipmapLod = vk::SamplerMipmapMode::eLinear;

    vk::DeviceSize size = 0;
    vk::ImageView imageView = nullptr;

    int width = 0;
    int height = 0;
    int channels = 4; // rgba
    int layers = 1;
    uint32_t mipLevels = 1;

    ~Image();

    void loadSTB(std::string path); // use stb_image.h to load most normal image formats
    void loadTextureCube(std::string path);

    void createSampler();

    void copyFrom(vk::CommandBuffer commandBuffer, const Buffer &buffer);
    void copyFrom(const Buffer &buffer);
    void resize(int width, int height);

    void generateMipmaps();
    void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void transitionImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

  private:
    std::string path;
    // create VkImage allocation
    void allocate();
    // destroy VkImage and VkImageView if they exist
    void deallocate();
    void createImageView();

    static auto hasStencilComponent(vk::Format format) -> bool;
};

} // namespace tat