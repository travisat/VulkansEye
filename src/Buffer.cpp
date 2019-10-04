#include "Buffer.hpp"
#include "macros.h"

Buffer::~Buffer()
{
    unmap();
    vmaDestroyBuffer(vulkan->allocator, buffer, allocation);
    Trace("Destroyed ", name, " at ", Timer::systemTime());
}

VkResult Buffer::map()
{
    VkResult result = vmaMapMemory(vulkan->allocator, allocation, &mapped);
    if (result != VK_SUCCESS)
    {
        Trace("Unable to map ", name, " at ", Timer::systemTime());
        return result;
    }
    Trace("Mapped ", name, " at ", Timer::systemTime());
    ismapped = true;
    return result;
}

void Buffer::unmap()
{
    if (ismapped)
    {
        vmaUnmapMemory(vulkan->allocator, allocation);
        Trace("Unmapped ", name, " at ", Timer::systemTime());
        mapped = nullptr;
        ismapped = false;
    }
}

VkResult Buffer::resize(VkDeviceSize s)
{
    VkResult result;
    if (!buffer) //buffer hasn't been allocated yet
    {
        result = allocate(s);
    }
    else if (s > size) //new buffer size is bigger than maximum buffer size
    {
        if (s > maxSize)
        {
            result = allocate(s);
        }
    }
    maxSize = size; //buffer requested is smaller than one already there, don't reallocate use old buffer space
    size = s;
    return VK_SUCCESS; //don't do anything with buffer and let new data be put in it
                       //todo determine if making buffer smaller is good idea
};

VkResult Buffer::allocate(VkDeviceSize s)
{
    size = s;
    maxSize = s;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = flags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = usage;

    VkResult result = vmaCreateBuffer(vulkan->allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
    if (result == VK_SUCCESS)
    {
        Trace("Allocated ", name, " with size ", size, " at ", Timer::systemTime());
    }
    return result;
}

void Buffer::deallocate()
{
    vmaDestroyBuffer(vulkan->allocator, buffer, allocation);
}

void Buffer::copyTo(Buffer &destination)
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

    vulkan->endSingleTimeCommands(commandBuffer);
}