#pragma once

#include "Config.h"
#include "Vertex.h"
#include "Model.hpp"

class Stage
{
public:
    tat::Vulkan *vulkan = nullptr;
    StageConfig *config;

    std::string name;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t vertexSize;
    uint32_t indexSize;
    Buffer vertexBuffer;
    Buffer indexBuffer;
    ImageType imageType;
    Image diffuse;
    Image normal;
    Image roughness;
    Image ambientOcclusion;
    VkSampler diffuseSampler;
    VkSampler normalSampler;
    VkSampler roughnessSampler;
    VkSampler ambientOcclusionSampler;
    std::vector<Buffer> uniformBuffers;
    std::vector<Buffer> uniformLights;
    std::vector<VkDescriptorSet> descriptorSets;

    ~Stage();
    void create();
    void loadMesh();
    void loadMaterial();

private:
    void loadImage(const std::string &path, ImageType type, Image &image, VkFormat format, VkSampler &sampler);
};