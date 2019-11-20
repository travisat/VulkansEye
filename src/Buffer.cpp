#include "Buffer.hpp"
#include "State.hpp"

#include <stdexcept>

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

namespace tat
{

Buffer::~Buffer()
{
    destroy();
}

void Buffer::create(VkDeviceSize s)
{
    destroy();

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

    allocId = engine.allocator.createBuffer(bufferInfo, allocInfo, buffer, &info);
    mapped = info.pMappedData;

    if constexpr (Debug::enableValidationLayers)
    {
        spdlog::info("Created Buffer {} : {}", allocId, name);
    }
}

void Buffer::destroy()
{
    auto &engine = State::instance().engine;
    engine.allocator.destroyBuffer(buffer, allocId);
    if constexpr (Debug::enableValidationLayers)
    {
        if (allocId >= 0)
        {
            spdlog::info("Destroyed Buffer {} : {}", allocId, name);
        }
    }
}

void Buffer::update(void *t, size_t s)
{
    auto &engine = State::instance().engine;
    if (s != size)
    {
        create(s);
    }
    void *data = nullptr;
    engine.allocator.mapMemory(allocId, &data);
    memcpy(data, t, s);
    engine.allocator.unmapMemory(allocId);
}

void Buffer::copyTo(Buffer &destination)
{
    auto &engine = State::instance().engine;
    // if destination buffer is a different size than source buffer reaclloate
    if (size != destination.size)
    {
        destination.create(size);
    }

    vk::CommandBuffer commandBuffer = engine.beginSingleTimeCommands();

    vk::BufferCopy copyRegion = {};
    copyRegion.size = size;
    commandBuffer.copyBuffer(buffer, destination.buffer, 1, &copyRegion);

    engine.endSingleTimeCommands(commandBuffer);
}

void Buffer::flush(size_t size, vk::DeviceSize offset)
{
    auto &engine = State::instance().engine;
    engine.allocator.flush(allocId, offset, size);
};

} // namespace tat