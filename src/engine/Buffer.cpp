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

    allocation = engine.allocator.create(bufferInfo, allocInfo, &info);
    buffer = std::get<vk::Buffer>(allocation->handle);
    mapped = info.pMappedData;

    if constexpr (Debug::enableValidationLayers)
    {
        spdlog::info("Created Buffer {} : {}", allocation->descriptor, name);
    }
}

void Buffer::destroy()
{
    if (buffer)
    {
        auto &engine = State::instance().engine;
        engine.allocator.destroy(allocation);
        if constexpr (Debug::enableValidationLayers)
        {
            if (allocation->descriptor >= 0)
            {
                spdlog::info("Destroyed Buffer {} : {}", allocation->descriptor, name);
            }
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
    
    memcpy(engine.allocator.map(allocation), t, s);
    engine.allocator.unmap(allocation);
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
    engine.allocator.flush(allocation, offset, size);
};

} // namespace tat