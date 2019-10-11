#pragma once

#include "Config.h"
#include "Vertex.h"

#include "Buffer.hpp"
#include "Image.hpp"
#include "Model.hpp"

class Stage
{
public:
    tat::Vulkan *vulkan = nullptr;
    StageConfig *config;

    std::string name;
    Model model;
    Buffer vertexBuffer;
    Buffer indexBuffer;
    std::vector<Buffer> uniformBuffers;
    std::vector<Buffer> uniformLights;
    glm::vec3 scale = glm::vec3(1.0f);

    std::vector<VkDescriptorSet> descriptorSets;

    void create();
    void createDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
    void createUniformBuffers();
    void updateUniformBuffer(uint32_t currentImage, UniformBufferObject &ubo, const UniformShaderObject *uso);

private:
};