#include "engine/Allocation.hpp"

#include <spdlog/spdlog.h>

namespace tat
{

auto Allocation::map() -> void *
{
    // Only map if allocation is valid
    if (descriptor >= 0 && allocation != nullptr && allocator != nullptr)
    {
        void *data;
        if (vmaMapMemory(allocator, allocation, &data) != VK_SUCCESS)
        {
            spdlog::error("Unable to map memory of allocation {}", descriptor);
            throw std::runtime_error("Unable to map memory of allocation");
        }
        return data;
    }

    spdlog::error("Unable to map memory, allocation {} not found", descriptor);
    throw std::runtime_error("Unable to map memory, allocation not found");
    return nullptr;
}

void Allocation::unmap()
{
    // Only unmap if allocation is valid
    if (descriptor >= 0 && allocation != nullptr && allocator != nullptr)
    {
        vmaUnmapMemory(allocator, allocation);
        return;
    }

    spdlog::error("Unable to unmap memory, allocation {} not found", descriptor);
    throw std::runtime_error("Unable to unmap memory, allocation not found");
}

void Allocation::flush(size_t size, size_t offset)
{
    // Only flush if allocation is valid
    if (descriptor >= 0 && allocation != nullptr && allocator != nullptr)
    {
        vmaFlushAllocation(allocator, allocation, offset, size);
        return;
    }

    spdlog::error("Unable to flush memory, allocation {} not found", descriptor);
    throw std::runtime_error("Unable to flush memory, allocation not found");
}

} // namespace tat