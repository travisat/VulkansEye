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

    vk::Image image{};
    auto result = vmaCreateImage(allocator, reinterpret_cast<VkImageCreateInfo *>(&imageInfo), &memInfo,
                                 reinterpret_cast<VkImage *>(&image), &allocation.allocation, allocInfo);

    if (result != VK_SUCCESS)
    {
        spdlog::error("Unable to create image. Error Code {}", result);
        throw std::runtime_error("Unable to create image");
    }

    allocation.handle = image;
    allocation.allocator = allocator;
    allocation.descriptor = accumulator;
    allocations.insert(std::make_pair(allocation.descriptor, allocation));
    ++accumulator;

    if constexpr (Debug::enableValidationLayers)
    { // only do this if validation layers are enabled
        Debug::setName(State::instance().engine.device.device,
                       std::get<vk::Image>(allocations.at(allocation.descriptor).handle),
                       fmt::format("Image {}", allocation.descriptor));
        spdlog::info("Allocated Image {}", allocation.descriptor);
    }

    return &allocations.at(allocation.descriptor);
}

auto Allocator::create(vk::BufferCreateInfo &bufferInfo, VmaAllocationCreateInfo &memInfo, VmaAllocationInfo *allocInfo)
    -> Allocation *
{
    Allocation allocation{};

    vk::Buffer buffer{};

    auto result = vmaCreateBuffer(allocator, reinterpret_cast<VkBufferCreateInfo *>(&bufferInfo), &memInfo,
                                  reinterpret_cast<VkBuffer *>(&buffer), &allocation.allocation, allocInfo);

    if (result != VK_SUCCESS)
    {
        spdlog::error("Unable to create buffer. Error Code {}", result);
        throw std::runtime_error("Unable to create buffer");
    }

    allocation.handle = buffer;
    allocation.allocator = allocator;
    allocation.descriptor = accumulator;
    allocations.insert(std::make_pair(allocation.descriptor, allocation));
    ++accumulator;

    if constexpr (Debug::enableValidationLayers)
    { // only do this if validation layers are enabled
        Debug::setName(State::instance().engine.device.device,
                       std::get<vk::Buffer>(allocations.at(allocation.descriptor).handle),
                       fmt::format("Buffer {}", allocation.descriptor));
        spdlog::info("Allocated buffer {}", allocation.descriptor);
    }

    return &allocations.at(allocation.descriptor);
}

void Allocator::destroy(Allocation *allocation)
{
    if (allocation != nullptr && allocation->descriptor >= 0 && allocation->allocation != nullptr &&
        allocation->allocator != nullptr)
    {
        auto descriptor = allocation->descriptor;
        if (std::holds_alternative<vk::Image>(allocation->handle))
        {
            vmaDestroyImage(allocator, std::get<vk::Image>(allocation->handle), allocation->allocation);

            if constexpr (Debug::enableValidationLayers)
            { // only do this if validation layers are enabled
                spdlog::info("Deallocated Image {}", descriptor);
            }
            allocations.erase(descriptor);
            allocation = nullptr;
            return;
        }
        if (std::holds_alternative<vk::Buffer>(allocation->handle))
        {
            vmaDestroyBuffer(allocator, std::get<vk::Buffer>(allocation->handle), allocation->allocation);

            if constexpr (Debug::enableValidationLayers)
            { // only do this if validation layers are enabled
                spdlog::info("Deallocated Buffer {}", descriptor);
            }
            allocations.erase(descriptor);
            allocation = nullptr;
        }
    }
}

} // namespace tat