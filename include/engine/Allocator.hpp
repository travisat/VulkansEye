#pragma once

#include <map>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <vk_mem_alloc.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{
class Allocator
{
  public:
    void create(vk::PhysicalDevice physicalDevice, vk::Device device);
    // destroys allocations, will free all memory held by allocations
    // even if buffer or image has not been destroyed
    void destroy();
    // creates image using vma and returns id of allocation
    auto createImage(vk::ImageCreateInfo &imageInfo, VmaAllocationCreateInfo &memInfo, vk::Image &image, VmaAllocationInfo *allocInfo = nullptr) -> int32_t;
    void destroyImage(vk::Image &image, int32_t allocationId);

    // same for buffer
    auto createBuffer(vk::BufferCreateInfo &bufferInfo, VmaAllocationCreateInfo &memInfo, vk::Buffer &buffer, VmaAllocationInfo *allocInfo = nullptr)
        -> int32_t;
    void destroyBuffer(vk::Buffer &buffer, int32_t allocationId);

    void mapMemory(int32_t allocId, void **data);
    void unmapMemory(int32_t allocId);
    void flush(int32_t allocId, size_t offset, size_t size);

  private:
    VmaAllocator allocator{};
    int32_t allocAccumulator = 1;
    std::map<int32_t, VmaAllocation> allocations;
};

} // namespace tat