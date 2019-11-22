#include "engine/Allocator.hpp"
#include "State.hpp"

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
        spdlog::error("Unable to create Allocator. Error code {}", result);
        throw std::runtime_error("Unable to create Allocator");
        return;
    }

    spdlog::info("Created Allocator");
}

void Allocator::destroy()
{
    if (!allocations.empty())
    {
        spdlog::warn("Destroying Allocator with {} allocations", allocations.size());
        for (auto &[descriptor, allocation] : allocations)
        {
            if (std::holds_alternative<vk::Image>(allocation.handle))
            {
                spdlog::warn("Destroying Image {}", descriptor);
                vmaDestroyImage(allocator, std::get<vk::Image>(allocation.handle), allocation.allocation);
            }
            else if (std::holds_alternative<vk::Buffer>(allocation.handle))
            {
                spdlog::warn("Destroying Buffer {}", descriptor);
                vmaDestroyBuffer(allocator, std::get<vk::Buffer>(allocation.handle), allocation.allocation);
            }
        }
        allocations.clear();
    }
    vmaDestroyAllocator(allocator);
    spdlog::info("Destroyed Allocator");
}

auto Allocator::create(vk::ImageCreateInfo &imageInfo, VmaAllocationCreateInfo &memInfo, VmaAllocationInfo *allocInfo)
    -> Allocation *
{
    Allocation allocation{};
    allocation.descriptor = accumulator;

    vk::Image image{};
    auto result = vmaCreateImage(allocator, reinterpret_cast<VkImageCreateInfo *>(&imageInfo), &memInfo,
                                 reinterpret_cast<VkImage *>(&image), &allocation.allocation, allocInfo);

    if (result != VK_SUCCESS)
    {
        spdlog::error("Unable to create image. Error Code {}", result);
        throw std::runtime_error("Unable to create image");
    }

    allocation.handle = image;

    ++accumulator;
    allocations.insert(std::make_pair(allocation.descriptor, allocation));

    if constexpr (Debug::enableValidationLayers)
    { // only do this if validation layers are enabled
        Debug::setName(State::instance().engine.device.device,
                       std::get<vk::Image>(allocations.at(allocation.descriptor).handle),
                       fmt::format("Image {}", allocation.descriptor));
        spdlog::info("Allocated Image {}", allocation.descriptor);
    }

    return &allocations.at(allocation.descriptor);
}

void Allocator::destroy(Allocation *allocation)
{
    if (allocations.find(allocation->descriptor) != allocations.end())
    {
        if (std::holds_alternative<vk::Image>(allocation->handle))
        {
            vmaDestroyImage(allocator, std::get<vk::Image>(allocation->handle), allocation->allocation);

            if constexpr (Debug::enableValidationLayers)
            { // only do this if validation layers are enabled
                spdlog::info("Deallocated Image {}", allocation->descriptor);
            }
            allocations.erase(allocation->descriptor);
            allocation = nullptr;
        }
        else if (std::holds_alternative<vk::Buffer>(allocation->handle))
        {
            vmaDestroyBuffer(allocator, std::get<vk::Buffer>(allocation->handle), allocation->allocation);

            if constexpr (Debug::enableValidationLayers)
            { // only do this if validation layers are enabled
                spdlog::info("Deallocated Buffer {}", allocation->descriptor);
            }
            allocations.erase(allocation->descriptor);
            allocation = nullptr;
        }
    }
}

auto Allocator::create(vk::BufferCreateInfo &bufferInfo, VmaAllocationCreateInfo &memInfo, VmaAllocationInfo *allocInfo)
    -> Allocation *
{
    Allocation allocation{};
    allocation.descriptor = accumulator;

    vk::Buffer buffer{};

    auto result = vmaCreateBuffer(allocator, reinterpret_cast<VkBufferCreateInfo *>(&bufferInfo), &memInfo,
                                  reinterpret_cast<VkBuffer *>(&buffer), &allocation.allocation, allocInfo);

    if (result != VK_SUCCESS)
    {
        spdlog::error("Unable to create buffer. Error Code {}", result);
        throw std::runtime_error("Unable to create buffer");
    }
    allocation.handle = buffer;

    ++accumulator;
    allocations.insert(std::make_pair(allocation.descriptor, allocation));

    if constexpr (Debug::enableValidationLayers)
    { // only do this if validation layers are enabled
        Debug::setName(State::instance().engine.device.device,
                       std::get<vk::Buffer>(allocations.at(allocation.descriptor).handle),
                       fmt::format("Buffer {}", allocation.descriptor));
        spdlog::info("Allocated buffer {}", allocation.descriptor);
    }

    return &allocations.at(allocation.descriptor);
}

auto Allocator::map(Allocation *allocation) -> void *
{
    void *data;
    if (allocations.find(allocation->descriptor) != allocations.end())
    {
        auto result = vmaMapMemory(allocator, allocation->allocation, &data);
        if (result != VK_SUCCESS)
        {
            spdlog::error("Unable to map memory of allocation {}", allocation->descriptor);
            throw std::runtime_error("Unable to map memory of allocation");
        }
        return data;
    }

    spdlog::error("Unable to map memory, Allocation Id {} not found", allocation->descriptor);
    throw std::runtime_error("Unable to map memory, allocation not found");
    return nullptr;
}

void Allocator::unmap(Allocation *allocation)
{
    if (allocations.find(allocation->descriptor) != allocations.end())
    {
        vmaUnmapMemory(allocator, allocation->allocation);
    }
    else
    {
        spdlog::error("Unable to unmap memory, Allocation Id {} not found", allocation->descriptor);
        throw std::runtime_error("Unable to unmap memory, allocation not found");
    }
}

void Allocator::flush(Allocation *allocation, size_t offset, size_t size)
{
    if (allocations.find(allocation->descriptor) != allocations.end())
    {
        vmaFlushAllocation(allocator, allocation->allocation, offset, size);
    }
    else
    {
        spdlog::error("Unable to flush memory, Allocation Id {} not found", allocation->descriptor);
        throw std::runtime_error("Unable to flush memory, allocation not found");
    }
}

} // namespace tat