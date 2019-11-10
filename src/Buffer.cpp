#include "Buffer.hpp"
#include "vulkan/vulkan_core.h"
#include <stdexcept>

namespace tat
{

Buffer::Buffer()
{
    debugLogger = spdlog::get("debugLogger");
}

Buffer::~Buffer()
{
    if (buffer)
    {
        deallocate();
    }
}

void Buffer::resize(VkDeviceSize s)
{
    if (buffer)
    {
        deallocate();
    }
    allocate(s);
}

void Buffer::update(void *t, size_t s)
{
    if (s != size)
    {
        if (buffer)
        {
            deallocate();
        }
        allocate(s);
    }
    void *data = nullptr;
    vmaMapMemory(vulkan->allocator, allocation, &data);
    memcpy(data, t, s);
    vmaUnmapMemory(vulkan->allocator, allocation);
}

void Buffer::allocate(VkDeviceSize s)
{
    size = s;

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = size;
    bufferInfo.usage = flags;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memUsage;
    allocInfo.flags = memFlags;

    VmaAllocationInfo info{};

    auto result = vmaCreateBuffer(vulkan->allocator, reinterpret_cast<VkBufferCreateInfo *>(&bufferInfo), &allocInfo,
                                reinterpret_cast<VkBuffer *>(&buffer), &allocation, &info);

    if (result != VK_SUCCESS)
    {
        debugLogger->error("Unable to create buffer of size {}. Error code {}", size, result);
        throw std::runtime_error("Unable to create buffer");
        return;
    }
    mapped = info.pMappedData;
}

void Buffer::deallocate()
{
    vmaDestroyBuffer(vulkan->allocator, buffer, allocation);
}

void Buffer::copyTo(Buffer &destination)
{
    // if destination buffer is a different size than source buffer reaclloate
    if (size != destination.size)
    {
        if (destination.buffer)
        {
            destination.deallocate();
        }
        destination.allocate(size);
    }

    vk::CommandBuffer commandBuffer = vulkan->beginSingleTimeCommands();

    vk::BufferCopy copyRegion = {};
    copyRegion.size = size;
    commandBuffer.copyBuffer(buffer, destination.buffer, 1, &copyRegion);

    vulkan->endSingleTimeCommands(commandBuffer);
}

} // namespace tat