#pragma once

#include <map>
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

struct Allocation
{
    int32_t descriptor = -1;
    std::variant<vk::Image, vk::Buffer> handle;
    VmaAllocation allocation{};
};

class Allocator
{
  public:
    void create(vk::PhysicalDevice physicalDevice, vk::Device device);
    // destroys allocations, will free all memory held by allocations
    // even if buffer or image has not been destroyed
    void destroy();
    // creates image using vma and returns descriptor and pointer of allocation
    auto create(vk::ImageCreateInfo &imageInfo, VmaAllocationCreateInfo &memInfo,
                VmaAllocationInfo *allocInfo = nullptr) -> Allocation *;

    // same for buffer
    auto create(vk::BufferCreateInfo &bufferInfo, VmaAllocationCreateInfo &memInfo,
                VmaAllocationInfo *allocInfo = nullptr) -> Allocation *;

    void destroy(Allocation *allocation);

    auto map(Allocation *allocation) -> void *;
    void unmap(Allocation *allocation);
    void flush(Allocation *allocation, size_t offset, size_t size);

  private:
    VmaAllocator allocator{};
    int32_t accumulator = 1;
    std::map<int32_t, Allocation> allocations{};
};

} // namespace tat