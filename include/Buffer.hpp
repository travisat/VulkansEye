#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <spdlog/spdlog.h>
#include <memory>

#include "Vertex.hpp"
namespace tat
{

class Buffer
{
  public:
    // required settings
    vk::BufferUsageFlags flags{};
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_UNKNOWN;
    VmaAllocationCreateFlags memFlags = 0;

    // created values
    vk::Buffer buffer = nullptr;
    void *mapped = nullptr;

    Buffer();
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
    std::shared_ptr<spdlog::logger> debugLogger;
    VmaAllocation allocation{};
    vk::DeviceSize size = 0;
};

} // namespace tat
