#include <filesystem>
#include <stdexcept>

#include <gli/gli.hpp>

#include "Image.hpp"
#include "vulkan/vulkan.hpp"

namespace tat
{

Image::Image(const std::shared_ptr<Vulkan> &vulkan)
{
    this->vulkan = vulkan;
    debugLogger = spdlog::get("debugLogger");

    // image createinfo defaults
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.format = vk::Format::eR8G8B8A8Unorm;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.usage = vk::ImageUsageFlagBits::eTransferSrc;
    imageInfo.extent = vk::Extent3D(0, 0, 1);
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.arrayLayers = 1;
    imageInfo.mipLevels = 1;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;

    memUsage = VMA_MEMORY_USAGE_UNKNOWN;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    currentLayout = vk::ImageLayout::eUndefined;

    // imageview createinfo defaults
    imageViewInfo.viewType = vk::ImageViewType::e2D;

    imageViewInfo.format = vk::Format::eR8G8B8A8Unorm;
    ;
    imageViewInfo.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.layerCount = 1;

    // sampler createinfo defaults
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.F;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
}

Image::~Image()
{
    if (image)
    {
        vmaDestroyImage(vulkan->allocator, image, allocation);
    }
    if (imageView)
    {
        vulkan->device.destroyImageView(imageView);
    }
    if (sampler)
    {
        vulkan->device.destroySampler(sampler);
    }
}

void Image::load(const std::string &path)
{
    this->path = path;
    if (!std::filesystem::exists(path))
    {
        debugLogger->warn("Image {} does not exist. Using default image", path);
        auto config = MaterialConfig{};
        this->path = config.diffuse;
    }
    gli::texture texture = gli::load(this->path);

    // this really isn't out of range, the enum in gli is weird
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-constant-out-of-range-compare"
    if (texture.target() == gli::TARGET_INVALID) // NOLINT
#pragma clang diagnostic pop
    {
        debugLogger->warn("Unable to load {}. Using default image", this->path);
        auto config = MaterialConfig{};
        texture = gli::load(config.diffuse);
    }

    Buffer stagingBuffer{};
    stagingBuffer.vulkan = vulkan;
    stagingBuffer.flags = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    stagingBuffer.update(texture.data(), texture.size());

    imageInfo.extent.width = texture.extent().x;
    imageInfo.extent.height = texture.extent().y;
    imageInfo.extent.depth = texture.extent().z;
    imageInfo.mipLevels = static_cast<uint32_t>(texture.levels());
    if (texture.target() == gli::TARGET_CUBE)
    {
        imageInfo.arrayLayers = texture.faces();
    }
    else
    {
        imageInfo.arrayLayers = texture.layers();
    }
    imageInfo.format = static_cast<vk::Format>(texture.format());

    allocate();

    transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    imageViewInfo.image = image;
    imageViewInfo.format = imageInfo.format;
    imageViewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
    imageViewInfo.subresourceRange.layerCount = imageInfo.arrayLayers;
    imageView = vulkan->device.createImageView(imageViewInfo);

    vk::CommandBuffer commandBuffer = vulkan->beginSingleTimeCommands();
    std::vector<vk::BufferImageCopy> bufferCopyRegions;

    // loop through faces/mipLevels in gli loaded texture and create regions to
    // copy
    uint32_t offset = 0;
    for (uint32_t layer = 0; layer < imageViewInfo.subresourceRange.layerCount; layer++)
    {
        for (uint32_t level = 0; level < imageInfo.mipLevels; level++)
        {
            auto extent(texture.extent(level));

            vk::BufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            bufferCopyRegion.imageSubresource.mipLevel = level;
            bufferCopyRegion.imageSubresource.baseArrayLayer = layer;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = extent.x;
            bufferCopyRegion.imageExtent.height = extent.y;
            bufferCopyRegion.imageExtent.depth = extent.z;
            bufferCopyRegion.bufferOffset = offset;

            bufferCopyRegions.push_back(bufferCopyRegion);

            offset += static_cast<uint32_t>(texture.size(level));
        }
    }

    vk::ImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = imageInfo.mipLevels;
    subresourceRange.layerCount = imageViewInfo.subresourceRange.layerCount;

    // copy gli loaded texture to image using regious created
    commandBuffer.copyBufferToImage(stagingBuffer.buffer, image, vk::ImageLayout::eTransferDstOptimal,
                                    static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

    vulkan->endSingleTimeCommands(commandBuffer);

    transitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    debugLogger->info("Loaded Image {}", this->path);
}

void Image::createSampler()
{
    sampler = vulkan->device.createSampler(samplerInfo);
}

void Image::allocate()
{
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memUsage;

    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    currentLayout = vk::ImageLayout::eUndefined;

    auto result = vmaCreateImage(vulkan->allocator, reinterpret_cast<VkImageCreateInfo *>(&imageInfo), &allocInfo,
                                 reinterpret_cast<VkImage *>(&image), &allocation, nullptr);

    if (result != VK_SUCCESS)
    {
        debugLogger->error("Unable to create image {}. Error Code {}", path, result);
        throw std::runtime_error("Unable to create image");
        return;
    }
}

void Image::deallocate()
{
    if (image)
    {
        vmaDestroyImage(vulkan->allocator, image, allocation);
    }
    if (imageView)
    {
        vulkan->device.destroyImageView(imageView);
    }
}

void Image::resize(int width, int height)
{
    if (imageInfo.extent != vk::Extent3D(width, height, imageInfo.extent.depth))
    {
        if (imageView)
        {
            vulkan->device.destroyImageView(imageView);
            imageView = nullptr;
        }
        deallocate();
        imageInfo.extent = vk::Extent3D(width, height, imageInfo.extent.depth);

        vk::ImageLayout tempLayout = currentLayout;
        allocate();
        if (tempLayout != vk::ImageLayout::eUndefined)
        {
            transitionImageLayout(vk::ImageLayout::eUndefined, tempLayout);
        }
        imageViewInfo.image = image;
        imageViewInfo.format = imageInfo.format;
        imageViewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
        imageViewInfo.subresourceRange.layerCount = imageInfo.arrayLayers;
        imageView = vulkan->device.createImageView(imageViewInfo);
    }
}

void Image::transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::CommandBuffer commandBuffer = vulkan->beginSingleTimeCommands();
    transitionImageLayout(commandBuffer, oldLayout, newLayout);
    vulkan->endSingleTimeCommands(commandBuffer);
}

void Image::transitionImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{

    vk::ImageMemoryBarrier barrier = {};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    barrier.subresourceRange.aspectMask = imageViewInfo.subresourceRange.aspectMask;

    if (imageInfo.format == vk::Format::eD32SfloatS8Uint || imageInfo.format == vk::Format::eD24UnormS8Uint)
    {
        barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = imageInfo.mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = imageInfo.arrayLayers;

    vk::PipelineStageFlags sourceStage = vk::PipelineStageFlagBits::eAllCommands;
    vk::PipelineStageFlags destinationStage = vk::PipelineStageFlagBits::eAllCommands;

    // https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanTools.cpp
    // https://github.com/jherico/Vulkan/blob/cpp/base/vks/vku.hpp
    switch (oldLayout)
    {
    case vk::ImageLayout::eUndefined:
        break;
    case vk::ImageLayout::eGeneral:
    case vk::ImageLayout::eTransferDstOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        break;
    case vk::ImageLayout::eTransferSrcOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        break;
    case vk::ImageLayout::eColorAttachmentOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        break;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        break;
    case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead;
        break;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        break;
    default:
        debugLogger->warn("Warning unsupported layout transition");
        break;
    }

    switch (newLayout)
    {
    case vk::ImageLayout::eUndefined:
        break;
    case vk::ImageLayout::eGeneral:
    case vk::ImageLayout::eTransferDstOptimal:
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        break;
    case vk::ImageLayout::eTransferSrcOptimal:
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        break;
    case vk::ImageLayout::eColorAttachmentOptimal:
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        break;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        barrier.dstAccessMask = barrier.dstAccessMask | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        break;
    case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead;
        break;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if (!barrier.srcAccessMask)
        {
            barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
        }
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        break;
    default:
        // Other source layouts aren't handled (yet)
        debugLogger->warn("Warning unsupported layout transition");
        break;
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);

    currentLayout = newLayout;
} // namespace tat

} // namespace tat