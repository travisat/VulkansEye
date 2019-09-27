#pragma once

#include <gli/gli.hpp>

#include "Image.h"
#include "Buffer.h"
#include "Camera.h"

class Skybox
{
public:
    Skybox::Skybox(State *_state, Camera *_camera, std::string _texturePath)
        : state(_state), camera(_camera), texturePath(_texturePath){};

    ~Skybox();

    void create();
    void cleanup();
    void recreate();

    void updateUniformBuffer(uint32_t currentImage);

    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<Buffer *> uniformBuffers;
    
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

private:
    State *state;
    Image *cubeMap;
    VkSampler sampler;

    VkDescriptorSetLayout descriptorSetLayout;
    std::string texturePath;

    Camera *camera;

    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorSets();
    void createPipeline();
};