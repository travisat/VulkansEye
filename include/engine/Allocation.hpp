#pragma once

#include <cstdint>
#include <variant>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <vk_mem_alloc.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{

using Handle = std::variant<vk::Image, vk::Buffer>;

class Allocation
{
  public:
    int32_t descriptor = -1;
    Handle handle;
    VmaAllocator allocator = nullptr;
    VmaAllocation allocation = nullptr;

    auto map() -> void *;
    void unmap();
    void flush(size_t size = VK_WHOLE_SIZE, size_t offset = 0);

    auto isImage() -> bool
    {
        return std::holds_alternative<vk::Image>(handle);
    };

    auto isBuffer() -> bool
    {
        return std::holds_alternative<vk::Buffer>(handle);
    };

  private:
};

} // namespace tat