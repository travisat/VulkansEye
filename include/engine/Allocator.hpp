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

#include "engine/Allocation.hpp"

namespace tat
{

using HandleCreateInfo = std::variant<vk::ImageCreateInfo, vk::BufferCreateInfo>;

class Allocator
{
  public:
    void create(vk::PhysicalDevice physicalDevice, vk::Device device);
    // destroys allocations, will free all memory held by allocations
    // even if buffer or image has not been destroyed
    void destroy();

    // creates handle using vma and returns pointer to allocation
    auto create(HandleCreateInfo createInfo, VmaAllocationCreateInfo &memInfo, VmaAllocationInfo *allocInfo = nullptr)
        -> Allocation *;

    // destroys allocation
    void destroy(Allocation *allocation);

  private:
    VmaAllocator allocator{};
    int32_t accumulator = 1;
    std::map<int32_t, Allocation> allocations{};
};

} // namespace tat