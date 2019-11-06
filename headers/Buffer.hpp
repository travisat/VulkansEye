#pragma once

#ifdef WINDOWS
#include <vcruntime.h>
#endif

#include "Timer.hpp"
#include "Vertex.hpp"
#include "Vulkan.hpp"
namespace tat
{

class Buffer
{
  public:
    // required settings
    Vulkan *vulkan = nullptr;
    vk::BufferUsageFlags flags{};
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_UNKNOWN;
    VmaAllocationCreateFlags memFlags = 0;

    // created values
    vk::Buffer buffer = nullptr;
    void *mapped = nullptr;

    ~Buffer();

    void allocate(VkDeviceSize s);
    void deallocate();

    // updates buffer to contents
    void update(void *t, size_t s);

    void resize(vk::DeviceSize s);
    void copyTo(Buffer &destination);
    auto getSize() -> vk::DeviceSize
    {
        return size;
    };

    void flush(size_t size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0)
    {
        vmaFlushAllocation(vulkan->allocator, allocation, offset, size);
    };

  private:
    VmaAllocation allocation{};
    vk::DeviceSize size = 0;
};

} // namespace tat
