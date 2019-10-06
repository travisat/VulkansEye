#include "Actor.hpp"
#include "helpers.h"

void Actor::create()
{
    //TODO implement using path to load model
    name = config->name;
    position = config->position;
    scale = config->scale;

    model.vulkan = vulkan;
    model.config =&config->modelConfig;
    model.create();

    createBuffers();
}

void Actor::createBuffers()
{
    //copy buffers to gpu only memory
    Buffer stagingBuffer{};
    stagingBuffer.vulkan = vulkan;
    stagingBuffer.flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    stagingBuffer.name = "scene staging buffer";

    stagingBuffer.load(model.vertices);
    vertexBuffer.vulkan = vulkan;
    vertexBuffer.flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBuffer.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    vertexBuffer.name = "scene/vertex";
    stagingBuffer.copyTo(vertexBuffer);

    stagingBuffer.load(model.indices);
    indexBuffer.vulkan = vulkan;
    indexBuffer.flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBuffer.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    indexBuffer.name = "scene/index";
    stagingBuffer.copyTo(indexBuffer);
}