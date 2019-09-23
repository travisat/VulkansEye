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

    void create();
    void cleanup();
    void recreate();

    void updateUniformBuffer(uint32_t currentImage);

    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<Buffer *> uniformBuffers;

    Mesh *mesh;
    Material *material;

    uint32_t vertexOffset;
    uint32_t indexOffset;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

private:
    State *state;
    VkDescriptorSetLayout descriptorSetLayout;
    std::string meshPath;
    std::string materialPath;


    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorSets();
    void createPipeline();
};