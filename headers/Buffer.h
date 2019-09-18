#pragma once

#include "Helpers.h"
#include "State.h"
#include "Vertex.h"

class Buffer
{
public:
    Buffer(State *state, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage);
    ~Buffer();
   
    void resize(VkDeviceSize size);
    void loadImage(stbi_uc *pixels);
    void loadImages(stbi_uc **pixels, VkDeviceSize size, uint32_t count);
    void load(std::vector<Vertex> const &vertices);
    void load(std::vector<uint32_t> const &indices);
    void copy(Buffer *destination);
    void update(UniformBufferObject &ubo);

    VkBuffer getBuffer() { return buffer; };
    VmaAllocation getAllocation() { return allocation; };
    VkDeviceSize getSize() { return bufferInfo.size; };

private:
    State *state;
    VkBuffer buffer;
    VmaAllocation allocation;
    VkBufferCreateInfo bufferInfo;
    VmaAllocationCreateInfo allocInfo;
};