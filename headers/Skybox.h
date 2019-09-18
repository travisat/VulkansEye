#pragma once

#include "Helpers.h"
#include "Image.h"
#include "Buffer.h"
#include "Vertex.h"

class Skybox
{
public:
    Skybox(State *state, std::array<std::string, 6> paths);
    ~Skybox();
    void createDescriptorSets();
    void createPipeline();
    void createUniformBuffers();
    VkBuffer getVertexBuffer() { return vertexBuffer->getBuffer(); };
    VkBuffer getIndexBuffer() { return indexBuffer->getBuffer(); };
    Buffer *getUniformBuffer(uint32_t i) { return uniformBuffers[i]; };
    VkDeviceSize getVertexBufferSize() { return vertexBuffer->getSize(); };
    VkDeviceSize getIndexCount() { return indexCount; };
    VkDescriptorSet *getDescriptorSet(uint32_t i) { return &descriptorSets[i]; };

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

private:
    State *state;
    Image *image;
    VkImageView imageView;
    Buffer *vertexBuffer;
    Buffer *indexBuffer;
    VkSampler sampler;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<Buffer *> uniformBuffers;

    uint32_t indexCount;

    void createCube();
};