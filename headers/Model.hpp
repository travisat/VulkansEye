#pragma once

#include <cassert>
#include <unordered_map>

#include <tiny_obj_loader.h>

#include "Config.h"
#include "Timer.h"
#include "Vertex.h"

#include "Buffer.hpp"
#include "Image.hpp"

namespace tat
{

class Model
{
  public:
    // config values
    tat::Vulkan *vulkan = nullptr;
    ModelConfig *config;
    std::string name = "Uknown Model";

    // generated values
    // mesh properties
    glm::vec3 position = glm::vec3(0.0F);
    glm::vec3 rotation = glm::vec3(0.0F);
    glm::vec3 scale = glm::vec3(1.0F);

    uint32_t vertexSize = 0;
    uint32_t indexSize = 0;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Buffer vertexBuffer;
    Buffer indexBuffer;

    // color pipeline
    std::vector<VkDescriptorSet> colorSets;
    Image diffuse;
    Image normal;
    Image roughness;
    Image metallic;
    Image ao;
    Image displacement;
    VkSampler diffuseSampler;
    VkSampler normalSampler;
    VkSampler roughnessSampler;
    VkSampler metallicSampler;
    VkSampler aoSampler;
    VkSampler dispSampler;
    std::vector<Buffer> tescBuffers;
    std::vector<Buffer> teseBuffers;
    std::vector<Buffer> uniformLights;
    TessControl uTessControl = {};
    TessEval uTessEval = {};

    // shadow pipeline
    std::vector<VkDescriptorSet> shadowSets;
    Image *shadow;
    UniformShadow uShadow = {};
    VkSampler shadowSampler;
    std::vector<Buffer> shadowBuffers;
   

    ~Model();
    void create();
    void createColorSets(VkDescriptorPool pool, VkDescriptorSetLayout layout);
    void createShadowSets(VkDescriptorPool pool, VkDescriptorSetLayout layout);
    void createUniformBuffers();

  private:
    void loadMesh();
    void loadMaterial();
    void loadImage(const std::string &path, Image &image, VkSampler &sampler);
    static void loadObj(const std::string &path, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
};

} // namespace tat