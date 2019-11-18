#include "engine/Allocator.hpp"

#include <spdlog/spdlog.h>
#include <stdexcept>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace tat
{

void Allocator::create(vk::PhysicalDevice physicalDevice, vk::Device device)
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;

    auto result = vmaCreateAllocator(&allocatorInfo, &allocator);
    if (result != VK_SUCCESS)
    {
        spdlog::error("Unable to create Memory Allocator. Error code {}", result);
        throw std::runtime_error("Unable to create Memory Allocator");
        return;
    }
    spdlog::info("Created Memory Allocator");
}

void Allocator::destroy()
{
    if (!allocations.empty())
    {
        spdlog::info("Destroying allocator with {} allocations", allocations.size());
        for (auto &[id, alloc] : allocations)
        {
            vmaFreeMemory(allocator, alloc);
        }
        allocations.clear();
    }
    vmaDestroyAllocator(allocator);
}

auto Allocator::createImage(vk::ImageCreateInfo &imageInfo, VmaAllocationCreateInfo &memInfo, vk::Image &image,
                            VmaAllocationInfo *allocInfo) -> int32_t
{
    auto index = allocAccumulator;
    ++allocAccumulator;
    allocations.insert(std::make_pair(index, VmaAllocation{}));

    auto result = vmaCreateImage(allocator, reinterpret_cast<VkImageCreateInfo *>(&imageInfo), &memInfo,
                                 reinterpret_cast<VkImage *>(&image), &allocations.at(index), allocInfo);

    if (result != VK_SUCCESS)
    {
        spdlog::error("Unable to create image. Error Code {}", result);
        throw std::runtime_error("Unable to create image");
    }

    return index;
}

void Allocator::destroyImage(vk::Image &image, int32_t allocationId)
{
    if (allocations.find(allocationId) != allocations.end())
    {
        vmaDestroyImage(allocator, image, allocations.at(allocationId));
        allocations.erase(allocationId);
    }
    image = nullptr;
}

auto Allocator::createBuffer(vk::BufferCreateInfo &bufferInfo, VmaAllocationCreateInfo &memInfo, vk::Buffer &buffer,
                             VmaAllocationInfo *allocInfo) -> int32_t
{
    auto index = allocAccumulator;
    ++allocAccumulator;
    allocations.insert(std::make_pair(index, VmaAllocation{}));

    auto result = vmaCreateBuffer(allocator, reinterpret_cast<VkBufferCreateInfo *>(&bufferInfo), &memInfo,
                                  reinterpret_cast<VkBuffer *>(&buffer), &allocations.at(index), allocInfo);

    if (result != VK_SUCCESS)
    {
        spdlog::error("Unable to create buffer. Error Code {}", result);
        throw std::runtime_error("Unable to create buffer");
    }

    return index;
}

void Allocator::destroyBuffer(vk::Buffer &buffer, int32_t allocationId)
{
    if (allocations.find(allocationId) != allocations.end())
    {
        vmaDestroyBuffer(allocator, buffer, allocations.at(allocationId));
        allocations.erase(allocationId);
    }
    buffer = nullptr;
}

void Allocator::mapMemory(int32_t allocId, void **data)
{
    if (allocations.find(allocId) != allocations.end())
    {
        auto result = vmaMapMemory(allocator, allocations.at(allocId), data);
        if (result != VK_SUCCESS)
        {
            spdlog::error("Unable to map memory of allocation {}", allocId);
            throw std::runtime_error("Unable to map memory of allocation");
        }
    }
    else
    {
        spdlog::error("Unable to map memory, Allocation Id {} not found", allocId);
        throw std::runtime_error("Unable to map memory, allocation not found");
    }
}

void Allocator::unmapMemory(int32_t allocId)
{
    if (allocations.find(allocId) != allocations.end())
    {
        vmaUnmapMemory(allocator, allocations.at(allocId));
    }
    else
    {
        spdlog::error("Unable to unmap memory, Allocation Id {} not found", allocId);
        throw std::runtime_error("Unable to unmap memory, allocation not found");
    }
}

void Allocator::flush(int32_t allocId, size_t offset, size_t size)
{
    if (allocations.find(allocId) != allocations.end())
    {
        vmaFlushAllocation(allocator, allocations.at(allocId), offset, size);
    }
    else
    {
        spdlog::error("Unable to flush memory, Allocation Id {} not found", allocId);
        throw std::runtime_error("Unable to flush memory, allocation not found");
    }
}

} // namespace tat