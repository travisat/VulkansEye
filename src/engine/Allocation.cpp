#include "engine/Allocation.hpp"
#include "engine/Debug.hpp"

#include <spdlog/spdlog.h>

namespace tat
{

auto Allocation::map() -> void *
{
    void *data;
    if (descriptor >= 0 && allocation != nullptr && allocator != nullptr)
    {
        auto result = vmaMapMemory(allocator, allocation, &data);
        if (result != VK_SUCCESS)
        {
            spdlog::error("Unable to map memory of allocation {}", descriptor);
            throw std::runtime_error("Unable to map memory of allocation");
        }
        return data;
    }

    spdlog::error("Unable to map memory, Allocation Id {} not found", descriptor);
    throw std::runtime_error("Unable to map memory, allocation not found");
    return nullptr;
}

void Allocation::unmap()
{
    if (descriptor >= 0 && allocation != nullptr && allocator != nullptr)
    {
        vmaUnmapMemory(allocator, allocation);
    }
    else
    {
        spdlog::error("Unable to unmap memory, Allocation Id {} not found", descriptor);
        throw std::runtime_error("Unable to unmap memory, allocation not found");
    }
}

void Allocation::flush(size_t size, size_t offset)
{
    if (descriptor >= 0 && allocation != nullptr && allocator != nullptr)
    {
        vmaFlushAllocation(allocator, allocation, offset, size);
    }
    else
    {
        spdlog::error("Unable to flush memory, Allocation Id {} not found", descriptor);
        throw std::runtime_error("Unable to flush memory, allocation not found");
    }
}

} // namespace tat