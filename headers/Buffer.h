#pragma once

#include "Helpers.h"
#include "State.h"
#include "Vertex.h"

class Buffer
{
public:
    VkBuffer buffer;

    Buffer(State *state, VkDeviceSize size, VkBufferUsageFlags flags, VmaMemoryUsage usage);
    ~Buffer();

    void resize(VkDeviceSize size);
    void loadImage(stbi_uc *pixels);
    void loadImages(stbi_uc **pixels, VkDeviceSize size, uint32_t count);
    void load(std::vector<Vertex> const &vertices);
    void load(std::vector<uint32_t> const &indices);
    void copy(Buffer *destination);
    void update(UniformBufferObject &ubo);

    VkDeviceSize getSize() { return size; };

private:
    State *state;
    VmaAllocation *allocation;
    VkDeviceSize size;
    VkBufferUsageFlags flags;
    VmaMemoryUsage usage;
};