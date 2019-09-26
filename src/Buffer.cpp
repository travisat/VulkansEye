#include "Buffer.h"

Buffer::Buffer(State *state, VkDeviceSize size, VkBufferUsageFlags flags, VmaMemoryUsage usage)
{
    this->state = state;
    this->flags = flags;
    this->usage = usage;
    this->size = size;

    allocation = new VmaAllocation();

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = flags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = usage;

    vmaCreateBuffer(state->allocator, &bufferInfo, &allocInfo, &buffer, allocation, nullptr);
}

Buffer::~Buffer()
{
    vmaDestroyBuffer(state->allocator, buffer, *allocation);
}

//destroys data in buffer
void Buffer::resize(VkDeviceSize size)
{
    vmaDestroyBuffer(state->allocator, buffer, *allocation);

    allocation = new VmaAllocation();

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = flags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = usage;

    vmaCreateBuffer(state->allocator, &bufferInfo, &allocInfo, &buffer, allocation, nullptr);
}

void Buffer::loadImage(stbi_uc *pixels)
{
    void *data;
    vmaMapMemory(state->allocator, *allocation, &data);
    memcpy(data, pixels, static_cast<size_t>(size));
    vmaUnmapMemory(state->allocator, *allocation);

}

void Buffer::loadImages(stbi_uc *pixels[], VkDeviceSize size, uint32_t count)
{
    void *data;
    vmaMapMemory(state->allocator, *allocation, &data);

    for (uint32_t i = 0; i < count; i++)
    {
        memcpy(data, pixels[i], static_cast<size_t>(size));
    }
    vmaUnmapMemory(state->allocator, *allocation);
}

void Buffer::load(std::vector<Vertex> const &vertices)
{
    void *data;
    vmaMapMemory(state->allocator, *allocation, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(size));
    vmaUnmapMemory(state->allocator, *allocation);
}

void Buffer::load(void *texData, uint32_t size)
{
    void *data;
    vmaMapMemory(state->allocator, *allocation, &data);
    memcpy(data, texData, static_cast<size_t>(size));
    vmaUnmapMemory(state->allocator, *allocation);
}

void Buffer::load(std::vector<uint32_t> const &indices)
{
    void *data;
    vmaMapMemory(state->allocator, *allocation, &data);
    memcpy(data, indices.data(), static_cast<size_t>(size));
    vmaUnmapMemory(state->allocator, *allocation);
}

void Buffer::copy(Buffer *destination)
{
    VkCommandBuffer commandBuffer = state->beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, buffer, destination->buffer, 1, &copyRegion);
    state->endSingleTimeCommands(commandBuffer);
}

void Buffer::update(UniformBufferObject &ubo)
{
    void *data;
    vmaMapMemory(state->allocator, *allocation, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(state->allocator, *allocation);
}