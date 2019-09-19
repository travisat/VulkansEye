#pragma once

#include "Helpers.h"
#include "Image.h"
#include "Buffer.h"
#include "Vertex.h"
#include "Mesh.h"
#include "Material.h"

class Skybox
{
public:
    Skybox(State *state, std::string meshPath, std::string materialPath);
    ~Skybox();
    void createDescriptorSets();
    void createPipeline();
   
    void updateUniformBuffer(uint32_t currentImage);

    Buffer *getUniformBuffer(uint32_t i) { return uniformBuffers[i]; };
    VkDescriptorSet *getDescriptorSet(uint32_t i) { return &descriptorSets[i]; };

    Mesh *mesh;
    Material *material;

    uint32_t vertexOffset;
    uint32_t indexOffset;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

private:
    State *state;

    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<Buffer *> uniformBuffers;

};