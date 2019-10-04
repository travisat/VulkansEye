#pragma once

#include <gsl/gsl>

#include "Vulkan.hpp"
#include "Vertex.h"
#include "Light.hpp"
#include "Timer.h"

class Buffer
{
public:
    tat::Vulkan *vulkan;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkBufferUsageFlags flags = 0;
    VmaMemoryUsage usage = VMA_MEMORY_USAGE_UNKNOWN;

    std::string name = "Unkown";

    ~Buffer();

    template <typename T>
    void load(std::vector<T> const &v)
    {
        if (v.size() * sizeof(T) != size)
        {
            allocate(v.size() * sizeof(T));
        }
        void *data;
        vmaMapMemory(vulkan->allocator, allocation, &data);
        memcpy(data, v.data(), static_cast<size_t>(size));
        vmaUnmapMemory(vulkan->allocator, allocation);
    };

    template <typename T>
    void load(gsl::span<T> const &t)
    {
        //check size of buffer currently loaded and if different reallocatue buffer
        if (t.size() * sizeof(T) != size)
        {
            allocate(t.size() * sizeof(T));
        }

        //map buffer memory
        //copy over data to buffer
        //unmap memory
        void *data;
        vmaMapMemory(vulkan->allocator, allocation, &data);
        memcpy(data, t.data(), static_cast<size_t>(size));
        vmaUnmapMemory(vulkan->allocator, allocation);
    };

    void copyTo(Buffer &destination);

    //updates buffer to contents in T
    template <typename T>
    void update(const T &t)
    {
        if (sizeof(T) != size)
        {
            allocate(sizeof(T));
        }
        void *data;
        vmaMapMemory(vulkan->allocator, allocation, &data);
        memcpy(data, &t, static_cast<size_t>(size));
        vmaUnmapMemory(vulkan->allocator, allocation);
    };

    VkDeviceSize getSize() { return size; };

    VkResult resize(VkDeviceSize s);

    void *mapped = nullptr;
    VkResult map();
    void unmap();

    void flush(size_t size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
    {
        vmaFlushAllocation(vulkan->allocator, allocation, offset, size);
    };

private:
    VmaAllocation allocation{};
    VkDeviceSize size = 0;
    VkDeviceSize maxSize = 0;

    VkResult allocate(VkDeviceSize s);
    void deallocate();

    bool ismapped = false;
};