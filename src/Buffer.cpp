#include "Buffer.hpp"
#include "helpers.h"

namespace tat
{

Buffer::~Buffer()
{
    deallocate();
}

void Buffer::resize(VkDeviceSize s)
{
    if (buffer != nullptr)
    {
        deallocate();
    }
    allocate(s);
    // Trace("Resized ", name, " to ", s, " at ", Timer::systemTime());
}

void Buffer::allocate(VkDeviceSize s)
{
    size = s;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = flags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memUsage;
    allocInfo.flags = memFlags;

    VmaAllocationInfo info = {};

    CheckResult(vmaCreateBuffer(vulkan->allocator, &bufferInfo, &allocInfo, &buffer, &allocation, &info));
    mapped = info.pMappedData;
    // Trace("Allocated ", name, " with size ", size, " at ",
    // Timer::systemTime());
}

void Buffer::deallocate()
{
    // Trace("Deallocated ", name, " at ", Timer::systemTime());
    vmaDestroyBuffer(vulkan->allocator, buffer, allocation);
}

void Buffer::copyTo(Buffer &destination)
{
    // if destination buffer is a different size than source buffer reaclloate
    if (size != destination.size)
    {
        if (destination.buffer != nullptr)
        {
            destination.deallocate();
        }
        destination.allocate(size);
    }

    VkCommandBuffer commandBuffer = vulkan->beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, buffer, destination.buffer, 1, &copyRegion);

    CheckResult(vulkan->endSingleTimeCommands(commandBuffer));
    // Trace("Copied ", name, " to ", destination.name, " at ",
    // Timer::systemTime());
}

} // namespace tat