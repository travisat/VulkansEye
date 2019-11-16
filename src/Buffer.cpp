#include "Buffer.hpp"
#include "State.hpp"

#include <stdexcept>

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

namespace tat
{

Buffer::~Buffer()
{
    deallocate();
}

void Buffer::resize(VkDeviceSize s)
{
    allocate(s);
}

void Buffer::update(void *t, size_t s)
{
    auto &engine = State::instance().engine;
    if (s != size)
    {
        allocate(s);
    }
    void *data = nullptr;
    vmaMapMemory(engine->allocator, allocation, &data);
    memcpy(data, t, s);
    vmaUnmapMemory(engine->allocator, allocation);
}

void Buffer::allocate(VkDeviceSize s)
{
    deallocate();

    auto &engine = State::instance().engine;
    size = s;

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = size;
    bufferInfo.usage = flags;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memUsage;
    allocInfo.flags = memFlags;

    VmaAllocationInfo info{};

    auto result = vmaCreateBuffer(engine->allocator, reinterpret_cast<VkBufferCreateInfo *>(&bufferInfo), &allocInfo,
                                  reinterpret_cast<VkBuffer *>(&buffer), &allocation, &info);

    if (result != VK_SUCCESS)
    {
        spdlog::error("Unable to create buffer of size {}. Error code {}", size, result);
        throw std::runtime_error("Unable to create buffer");
        return;
    }
    mapped = info.pMappedData;
    allocated = true;
}

void Buffer::deallocate()
{
    if (allocated)
    {
        auto &state = State::instance();
        vmaDestroyBuffer(state.engine->allocator, buffer, allocation);
        allocated = false;
        buffer = nullptr;
    }
}

void Buffer::copyTo(Buffer &destination)
{
    auto &engine = State::instance().engine;
    // if destination buffer is a different size than source buffer reaclloate
    if (size != destination.size)
    {
        destination.allocate(size);
    }

    vk::CommandBuffer commandBuffer = engine->beginSingleTimeCommands();

    vk::BufferCopy copyRegion = {};
    copyRegion.size = size;
    commandBuffer.copyBuffer(buffer, destination.buffer, 1, &copyRegion);

    engine->endSingleTimeCommands(commandBuffer);
}

void Buffer::flush(size_t size, vk::DeviceSize offset)
{
    auto &state = State::instance();
    vmaFlushAllocation(state.engine->allocator, allocation, offset, size);
};

} // namespace tat