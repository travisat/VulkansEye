#pragma once

#include "Config.h"
#include "Vertex.h"

#include "Buffer.hpp"
#include "Image.hpp"
#include "Model.hpp"
#include "Backdrop.hpp"
#include "Player.hpp"

class Stage
{
public:
    tat::Vulkan *vulkan = nullptr;
    StageConfig *config;
    Player *player;

    Backdrop backdrop;
    Model model;
    Buffer vertexBuffer;
    Buffer indexBuffer;
    std::vector<Buffer> uniformBuffers;
    std::vector<Buffer> uniformLights;

    std::vector<VkDescriptorSet> descriptorSets;

    void create();
    void recreate(){ backdrop.recreate();};
    void cleanup(){backdrop.cleanup();};
    void createDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
    void createUniformBuffers();

private:
};