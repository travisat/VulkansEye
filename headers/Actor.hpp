#pragma once

#include "Config.h"

#include "Vulkan.hpp"
#include "Buffer.hpp"
#include "Model.hpp"

namespace tat
{

class Actor
{
public:
    //config values
    tat::Vulkan *vulkan = nullptr;
    ActorConfig *config;

    //generated values
    std::string name = "Unknown Actor";
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    Model model;
    Buffer vertexBuffer;
    Buffer indexBuffer;
    std::vector<Buffer> tescBuffers;
    std::vector<Buffer> teseBuffers;
    std::vector<Buffer> uniformLights;
    std::vector<VkDescriptorSet> descriptorSets;

    void create();
    void createDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorLayout);
    void createUniformBuffers();

private:
};

} //namespace tat