#pragma once

#include "Image.hpp"
#include "Buffer.hpp"
#include "Camera.hpp"
#include "Timer.h"

class Skybox
{
public:
    Skybox::Skybox(tat::Vulkan *_vulkan, Camera *_camera, std::string _texturePath)
        : vulkan(_vulkan), camera(_camera), texturePath(_texturePath){};

    ~Skybox();

    void create();
    void cleanup();
    void recreate();

    void draw(VkCommandBuffer commandBuffer, uint32_t currentImage);

    void updateUniformBuffer(uint32_t currentImage);

    std::vector<VkDescriptorSet> descriptorSets {};
    std::vector<Buffer> uniformBuffers {};

    tat::Vulkan *vulkan = nullptr;
    
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

private:
    Image cubeMap {};
    VkSampler sampler = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    std::string texturePath {};

    Camera *camera  = nullptr;

    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorSets();
    void createPipeline();
};