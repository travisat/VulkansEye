#pragma once

#include "Buffer.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Vulkan.hpp"
#include "Config.h"

class Model
{
public:
    Model(tat::Vulkan *vulkan, const ModelConfig &config); //for already loaded mesh/material

    tat::Vulkan *vulkan = nullptr;

    void draw(const VkCommandBuffer &commandBuffer);

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale =glm::vec3(1.0f);
    ModelType type= ModelType::unknown;
    std::string name = "Unknwon";

    Mesh mesh {};
    Material material {};

    std::vector<Buffer> uniformBuffers {};
    std::vector<Buffer> uniformLights{};
    std::vector<VkDescriptorSet> descriptorSets {};

    int32_t getId(){ return id;};
private:
    int32_t id = 0;
    
};