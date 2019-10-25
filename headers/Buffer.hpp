#pragma once

#include "Timer.h"
#include "Vertex.h"
#include "Vulkan.hpp"
#include <vcruntime.h>

namespace tat
{

class Buffer
{
  public:
    // required settings
    tat::Vulkan *vulkan = nullptr;
    VkBufferUsageFlags flags = 0;
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_UNKNOWN;
    VmaAllocationCreateFlags memFlags = 0;

    // created values
    VkBuffer buffer = VK_NULL_HANDLE;
    void *mapped = nullptr;

    ~Buffer();
    void allocate(VkDeviceSize s);
    void deallocate();

    // updates buffer to contents in vector
    template <typename T> void update(std::vector<T> const &v)
    {
        size_t s = v.size() * sizeof(v[0]);
        if (s != size)
        {
            if (buffer)
                deallocate();
            allocate(s);
        }
        void *data;
        vmaMapMemory(vulkan->allocator, allocation, &data);
        memcpy(data, v.data(), static_cast<size_t>(size));
        vmaUnmapMemory(vulkan->allocator, allocation);
    };

    // updates buffer to contents in vector
    template <typename T, size_t S> void update(std::array<T, S> const &a)
    {
        size_t s = S * sizeof(a[0]);
        if (s != size)
        {
            if (buffer)
                deallocate();
            allocate(s);
        }
        void *data;
        vmaMapMemory(vulkan->allocator, allocation, &data);
        memcpy(data, a.data(), static_cast<size_t>(size));
        vmaUnmapMemory(vulkan->allocator, allocation);
    };

    // updates buffer with unknown data of size s
    template <typename T> void update(T *t, size_t s)
    {
        if (s != size)
        {
            if (buffer)
                deallocate();
            allocate(s);
        }
        void *data;
        vmaMapMemory(vulkan->allocator, allocation, &data);
        memcpy(data, t, s);
        vmaUnmapMemory(vulkan->allocator, allocation);
    };

    void resize(VkDeviceSize s);
    void copyTo(Buffer &destination);
    auto getSize() -> VkDeviceSize
    {
        return size;
    };

    void flush(size_t size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
    {
        vmaFlushAllocation(vulkan->allocator, allocation, offset, size);
    };

  private:
    VmaAllocation allocation{};
    VkDeviceSize size = 0;

    bool ismapped = false;
};

} // namespace tat