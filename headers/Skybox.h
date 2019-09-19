#pragma once

#include "Helpers.h"
#include "Image.h"
#include "Buffer.h"
#include "Vertex.h"

class Skybox
{
public:
    Buffer *vertexBuffer;
    Buffer *indexBuffer;

    Skybox(State *state, std::array<std::string, 6> paths);
    ~Skybox();
    void createDescriptorSets();
    void createPipeline();
    void createUniformBuffers();
    void updateUniformBuffer(uint32_t currentImage);

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

    VkSampler sampler;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<Buffer *> uniformBuffers;

    uint32_t indexCount;

    void createCube();
};