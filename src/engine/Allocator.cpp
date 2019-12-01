#include "engine/Allocator.hpp"
#include "State.hpp"

#include <spdlog/spdlog.h>
#include <stdexcept>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace tat
{

inline auto isImageCreateInfo(const HandleCreateInfo &createInfo) -> bool
{
    return std::holds_alternative<vk::ImageCreateInfo>(createInfo);
}

inline auto isBufferCreateInfo(const HandleCreateInfo &createInfo) -> bool
{
    return std::holds_alternative<vk::BufferCreateInfo>(createInfo);
}

void Allocator::create(vk::PhysicalDevice physicalDevice, vk::Device device)
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;

    if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS)
    {
        spdlog::error("Unable to create Allocator");
        throw std::runtime_error("Unable to create Allocator");
    }

    if constexpr (Debug::enable)
    {
        spdlog::info("Created Allocator");
    }
}

void Allocator::destroy()
{
    if (!allocations.empty())
    {
        spdlog::warn("Destroying Allocator with {} allocations", allocations.size());
        for (auto &[descriptor, allocation] : allocations)
        {
            if (allocation.isImage())
            {
                spdlog::warn("Destroying Image {}", descriptor);
                vmaDestroyImage(allocator, std::get<vk::Image>(allocation.handle), allocation.allocation);
            }
            else if (allocation.isBuffer())
            {
                spdlog::warn("Destroying Buffer {}", descriptor);
                vmaDestroyBuffer(allocator, std::get<vk::Buffer>(allocation.handle), allocation.allocation);
            }
        }
        allocations.clear();
    }
    vmaDestroyAllocator(allocator);

    if constexpr (Debug::enable)
    {
        spdlog::info("Destroyed Allocator");
    }
}

auto Allocator::create(HandleCreateInfo createInfo, VmaAllocationCreateInfo &memInfo, VmaAllocationInfo *allocInfo)
    -> Allocation *
{
    if (isImageCreateInfo(createInfo))
    {
        Allocation allocation{};
        vk::Image image{};

        if (vmaCreateImage(allocator, reinterpret_cast<VkImageCreateInfo *>(&createInfo), &memInfo,
                           reinterpret_cast<VkImage *>(&image), &allocation.allocation, allocInfo) != VK_SUCCESS)
        {
            spdlog::error("Unable to create image");
            throw std::runtime_error("Unable to create image");
        }

        allocation.handle = image;
        allocation.allocator = allocator;
        allocation.descriptor = accumulator;
        allocations.insert(std::make_pair(allocation.descriptor, allocation));
        ++accumulator;

        if constexpr (Debug::enable)
        { // only do this if validation layers are enabled
            Debug::setName(State::instance().engine.device.device,
                           std::get<vk::Image>(allocations.at(allocation.descriptor).handle),
                           fmt::format("Image {}", allocation.descriptor));
            spdlog::info("Allocated Image {}", allocation.descriptor);
        }
        return &allocations.at(allocation.descriptor);
    }

    if (isBufferCreateInfo(createInfo))
    {
        Allocation allocation{};
        vk::Buffer buffer{};

        if (vmaCreateBuffer(allocator, reinterpret_cast<VkBufferCreateInfo *>(&createInfo), &memInfo,
                            reinterpret_cast<VkBuffer *>(&buffer), &allocation.allocation, allocInfo) != VK_SUCCESS)
        {
            spdlog::error("Unable to create buffer");
            throw std::runtime_error("Unable to create buffer");
        }

        allocation.handle = buffer;
        allocation.allocator = allocator;
        allocation.descriptor = accumulator;
        allocations.insert(std::make_pair(allocation.descriptor, allocation));
        ++accumulator;

        if constexpr (Debug::enable)
        {
            Debug::setName(State::instance().engine.device.device,
                           std::get<vk::Buffer>(allocations.at(allocation.descriptor).handle),
                           fmt::format("Buffer {}", allocation.descriptor));
            spdlog::info("Allocated buffer {}", allocation.descriptor);
        }

        return &allocations.at(allocation.descriptor);
    }

    return nullptr;
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
            allocations.erase(descriptor);
            allocation = nullptr;

            if constexpr (Debug::enable)
            {
                spdlog::info("Deallocated Image {}", descriptor);
            }

            return;
        }
        if (std::holds_alternative<vk::Buffer>(allocation->handle))
        {
            vmaDestroyBuffer(allocator, std::get<vk::Buffer>(allocation->handle), allocation->allocation);
            allocations.erase(descriptor);
            allocation = nullptr;

            if constexpr (Debug::enable)
            {
                spdlog::info("Deallocated Buffer {}", descriptor);
            }
        }
    }
}

} // namespace tat