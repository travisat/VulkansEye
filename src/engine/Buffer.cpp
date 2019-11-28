#include "engine/Buffer.hpp"
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

    size = s;

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = size;
    bufferInfo.usage = flags;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memUsage;
    allocInfo.flags = memFlags;

    VmaAllocationInfo info{};

    auto &allocator = State::instance().engine.allocator;
    allocation = allocator.create(bufferInfo, allocInfo, &info);

    buffer = std::get<vk::Buffer>(allocation->handle);
    mapped = info.pMappedData;

    if constexpr (Debug::enable)
    {
        spdlog::info("Created Buffer {} : {}", allocation->descriptor, name);
    }
}

void Buffer::destroy()
{
    if (buffer)
    {
        auto &allocator = State::instance().engine.allocator;
        allocator.destroy(allocation);
        buffer = nullptr;
    }
}

void Buffer::update(void *t, size_t s)
{
    if (s != size)
    {
        create(s);
    }

    memcpy(allocation->map(), t, s);
    allocation->unmap();
}

void Buffer::copyTo(Buffer &destination)
{
    // if destination buffer is a different size than source buffer reaclloate
    if (size != destination.size)
    {
        destination.create(size);
    }

    auto &engine = State::instance().engine;
    auto commandBuffer = engine.beginSingleTimeCommands();

    vk::BufferCopy copyRegion{};
    copyRegion.size = size;
    commandBuffer.copyBuffer(buffer, destination.buffer, 1, &copyRegion);

    engine.endSingleTimeCommands(commandBuffer);
}

void Buffer::flush(size_t size, vk::DeviceSize offset)
{
    allocation->flush(size, offset);
};

} // namespace tat