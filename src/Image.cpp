#include "Image.hpp"
#include "gli/target.hpp"
#include "helpers.hpp"
#include "vulkan/vulkan_core.h"
#include <stdexcept>

namespace tat
{

Image::Image()
{
    debugLogger = spdlog::get("debugLogger");
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

void Image::loadSTB(const std::string &path)
{
    this->path = path;
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

    int32_t defaultChannels = 4;
    format = vk::Format::eR8G8B8A8Unorm;

    stbi_info(path.c_str(), &width, &height, &channels);

    if (channels == 1 && vulkan->checkFormat(vk::Format::eR8Unorm))
    {
        defaultChannels = 1;
        format = vk::Format::eR8Unorm;
    }
    else if (channels == 2 && vulkan->checkFormat(vk::Format::eR8G8Unorm))
    {
        defaultChannels = 2;
        format = vk::Format::eR8G8Unorm;
    }
    else if (channels == 3 && vulkan->checkFormat(vk::Format::eR8G8B8Unorm))
    {
        defaultChannels = 3;
        format = vk::Format::eR8G8B8Unorm;
    }

    stbi_uc *pixels = stbi_load(path.c_str(), &width, &height, &channels, defaultChannels);
    channels = defaultChannels;
    size = width * height * channels;
    // TODO(travis) make this fall through and let default texture be used instead of halting
    if (pixels == nullptr)
    {
        debugLogger->error("Unable to load {}", path);
        throw std::runtime_error("Unable to load Image in Image::loadSTB");
    }

    Buffer stagingBuffer{};
    stagingBuffer.vulkan = vulkan;
    stagingBuffer.flags = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    stagingBuffer.update(pixels, size);

    stbi_image_free(pixels);

    layout = vk::ImageLayout::eTransferDstOptimal;
    allocate();
    copyFrom(stagingBuffer);

    generateMipmaps();
    debugLogger->info("Loaded Image {}", path);
}

void Image::loadGLI(const std::string &path)
{
    this->path = path;
    gli::texture texture = gli::load(path);
    // TODO(travis) make this fall through and let default texture be used instead of halting
    // this really isn't out of range, the enum in gli is done weird
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-constant-out-of-range-compare"
    if (texture.target() == gli::TARGET_INVALID) // NOLINT
#pragma clang diagnostic pop
    {

        debugLogger->error("Unable to load {}", path);
        throw std::runtime_error("Unable to load Image in Image::loadGLI");
    }

    Buffer stagingBuffer{};
    stagingBuffer.vulkan = vulkan;
    stagingBuffer.flags = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    stagingBuffer.update(texture.data(), texture.size());

    width = texture.extent().x;
    height = texture.extent().y;
    mipLevels = static_cast<uint32_t>(texture.levels());
    if (texture.target() == gli::TARGET_CUBE)
    {
        layers = texture.faces();
    }
    else
    {
        layers = texture.layers();
    }
    layout = vk::ImageLayout::eTransferDstOptimal;
    format = static_cast<vk::Format>(texture.format());
    allocate();

    vk::CommandBuffer commandBuffer = vulkan->beginSingleTimeCommands();
    std::vector<vk::BufferImageCopy> bufferCopyRegions;

    // loop through faces/mipLevels in gli loaded texture and create regions to
    // copy
    uint32_t offset = 0;
    for (uint32_t layer = 0; layer < layers; layer++)
    {
        for (uint32_t level = 0; level < mipLevels; level++)
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
    subresourceRange.levelCount = mipLevels;
    subresourceRange.layerCount = layers;

    // copy gli loaded texture to image using regious created
    commandBuffer.copyBufferToImage(stagingBuffer.buffer, image, vk::ImageLayout::eTransferDstOptimal,
                                    static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

    vulkan->endSingleTimeCommands(commandBuffer);

    transitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    debugLogger->info("Loaded Image {}", path);
}

void Image::createSampler()
{
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = magFilter;
    samplerInfo.minFilter = minFilter;
    samplerInfo.addressModeU = addressModeU;
    samplerInfo.addressModeV = addressModeV;
    samplerInfo.addressModeW = addressModeW;
    samplerInfo.anisotropyEnable = anistropyEnable;
    samplerInfo.maxAnisotropy = maxAnisotropy;
    samplerInfo.borderColor = borderColor;
    samplerInfo.unnormalizedCoordinates = unnormalizedCoordinates;
    samplerInfo.compareEnable = compareEnable;
    samplerInfo.compareOp = compareOp;
    samplerInfo.mipmapMode = mipmapLod;
    samplerInfo.maxLod = static_cast<float>(mipLevels);

    sampler = vulkan->device.createSampler(samplerInfo);
}

void Image::allocate()
{
    vk::ImageCreateInfo imageInfo = {};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = layers;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = imageUsage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.flags = flags;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memUsage;

    auto result = vmaCreateImage(vulkan->allocator, reinterpret_cast<VkImageCreateInfo *>(&imageInfo), &allocInfo,
                               reinterpret_cast<VkImage *>(&image), &allocation, nullptr);

    if (result != VK_SUCCESS)
    {
        debugLogger->error("Unable to create image {}. Error Code {}", path, result);
        throw std::runtime_error("Unable to create image");
        return;
    }

    if (layout != vk::ImageLayout::eUndefined)
    {
        transitionImageLayout(vk::ImageLayout::eUndefined, layout);
    }
    createImageView();
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

void Image::createImageView()
{
    vk::ImageViewCreateInfo viewInfo = {};
    viewInfo.image = image;
    viewInfo.viewType = viewType;
    viewInfo.format = format;
    viewInfo.subresourceRange = {aspect, 0, 1, 0, 1};
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.layerCount = layers;

    imageView = vulkan->device.createImageView(viewInfo);
}

void Image::copyFrom(const Buffer &buffer)
{
    vk::CommandBuffer commandBuffer = vulkan->beginSingleTimeCommands();
    copyFrom(commandBuffer, buffer);
    vulkan->endSingleTimeCommands(commandBuffer);
}

void Image::copyFrom(vk::CommandBuffer commandBuffer, const Buffer &buffer)
{

    vk::BufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layers;

    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};

    commandBuffer.copyBufferToImage(buffer.buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
}

void Image::resize(int width, int height)
{
    if (this->width != width || this->height != height)
    {
        if (imageView)
        {
            vulkan->device.destroyImageView(imageView);
            imageView = nullptr;
        }
        deallocate();
        this->width = width;
        this->height = height;
        allocate();
    }
}

void Image::generateMipmaps()
{
    auto formatProperties = vulkan->physicalDevice.getFormatProperties(format);

    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
    {
        debugLogger->warn("Warning {} does not support linear blitting", path);
        return;
    }

    vk::CommandBuffer commandBuffer = vulkan->beginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier = {};
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = width;
    int32_t mipHeight = height;

    for (uint32_t i = 1; i < mipLevels; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, 0,
                                      nullptr, 0, nullptr, 1, &barrier);

        vk::ImageBlit blit = {};
        blit.srcOffsets[0] = vk::Offset3D{0, 0, 0};
        blit.srcOffsets[1] = vk::Offset3D{mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = vk::Offset3D{0, 0, 0};
        blit.dstOffsets[1] = vk::Offset3D{mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image,
                                vk::ImageLayout::eTransferDstOptimal, 1, &blit, vk::Filter::eLinear);

        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                                      {}, 0, nullptr, 0, nullptr, 1, &barrier);

        if (mipWidth > 1)
        {
            mipWidth /= 2;
        }
        if (mipHeight > 1)
        {
            mipHeight /= 2;
        }
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
                                  0, nullptr, 0, nullptr, 1, &barrier);

    vulkan->endSingleTimeCommands(commandBuffer);
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

    barrier.subresourceRange.aspectMask = aspect;

    if (hasStencilComponent(format))
    {
        barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layers;

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

    layout = newLayout;
} // namespace tat

auto Image::hasStencilComponent(vk::Format format) -> bool
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

} // namespace tat