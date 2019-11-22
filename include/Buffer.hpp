#pragma once

#include <memory>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <vk_mem_alloc.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "Vertex.hpp"
#include "engine/Allocator.hpp"
namespace tat
{

class Buffer
{
  public:
    Buffer() = default;
    ~Buffer();

    // don't use uniquebuffer, vma is used

    vk::Buffer buffer = nullptr;
    void *mapped = nullptr;

    vk::BufferUsageFlags flags{};
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_UNKNOWN;
    VmaAllocationCreateFlags memFlags = 0;
    std::string name = "";

    void create(VkDeviceSize s);
    void destroy();

    // updates buffer to contents
    void update(void *t, size_t s);

    void copyTo(Buffer &destination);
    auto getSize() -> vk::DeviceSize
    {
        return size;
    };

    void flush(size_t size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);

  private:
    Allocation *allocation = nullptr;
    vk::DeviceSize size = 0;
};

} // namespace tat
