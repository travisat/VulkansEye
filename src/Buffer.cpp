#include "Buffer.hpp"
#include "macros.h"

Buffer::~Buffer()
{
    vmaDestroyBuffer(vulkan->allocator, buffer, allocation);
    Trace("Destroyed ", name, " at ", Timer::systemTime());
}

VkResult Buffer::resize(VkDeviceSize s)
{
    VkResult result;
    if (buffer) 
    {
        deallocate();
    }
    result = allocate(s);
    if( result != VK_SUCCESS)
    {
          Trace("Unable to resize ", name, " to ", s, " at ", Timer::systemTime());
          return result;
    }
    Trace("Resized ", name, " to ", s, " at ", Timer::systemTime());
    return result;
};

VkResult Buffer::allocate(VkDeviceSize s)
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

    VkResult result = vmaCreateBuffer(vulkan->allocator, &bufferInfo, &allocInfo, &buffer, &allocation, &info);
    if (result != VK_SUCCESS)
    {
        Trace("Unable to llocate ", name, " with size ", size, " at ", Timer::systemTime());
    }
    mapped = info.pMappedData;
    Trace("Allocated ", name, " with size ", size, " at ", Timer::systemTime());
    return result;
}

void Buffer::deallocate()
{
    Trace("Deallocated ", name, " at ", Timer::systemTime());
    vmaDestroyBuffer(vulkan->allocator, buffer, allocation);
}

VkResult Buffer::copyTo(Buffer &destination)
{
    //if destination buffer is a different size than source buffer reaclloate
    if (size != destination.size)
    {
        destination.allocate(size);
    }

    VkCommandBuffer commandBuffer = vulkan->beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, buffer, destination.buffer, 1, &copyRegion);

    VkResult result = vulkan->endSingleTimeCommands(commandBuffer);

    if (result != VK_SUCCESS)
    {
        Trace("Unable to copy ", name, " to ", destination.name, " at ", Timer::systemTime());
        return result;
    }
    Trace("Copied ", name, " to ", destination.name, " at ", Timer::systemTime());
    return result;
}