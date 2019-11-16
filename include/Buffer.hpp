#pragma once
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <vk_mem_alloc.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <memory>

#include "Vertex.hpp"
namespace tat
{

class Buffer
{
  public:
    vk::BufferUsageFlags flags{};
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_UNKNOWN;
    VmaAllocationCreateFlags memFlags = 0;

    //don't use uniquebuffer, vma is used for memory for it
    vk::Buffer buffer = nullptr;
    void *mapped = nullptr;

    Buffer() = default;
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

    void flush(size_t size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    

  private:
    bool allocated = false;
    VmaAllocation allocation{};
    vk::DeviceSize size = 0;
};

} // namespace tat
