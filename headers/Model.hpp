#pragma once

#include <cassert>
#include <unordered_map>

#include "Config.h"
#include "Materials.hpp"
#include "Meshes.hpp"
#include "Timer.h"
#include "Vertex.h"

#include "Buffer.hpp"
#include "Image.hpp"
#include "vulkan/vulkan.hpp"

namespace tat
{

class Model
{
  public:
    // config values
    tat::Vulkan *vulkan = nullptr;
    ModelConfig *config;
    Materials *materials;
    Meshes *meshes;

    std::string name = "Uknown Model";

    // mesh properties
    Mesh *mesh;
    glm::vec3 position = glm::vec3(0.0F);
    glm::vec3 rotation = glm::vec3(0.0F);
    glm::vec3 scale = glm::vec3(1.0F);

    // color pipeline
    Material *material;
    std::vector<vk::DescriptorSet> colorSets;
    std::vector<Buffer> tescBuffers;
    std::vector<Buffer> teseBuffers;
    std::vector<Buffer> uniformLights;
    TessControl uTessControl = {};
    TessEval uTessEval = {};
    Image *irradianceMap;
    Image *radianceMap;

    // shadow pipeline
    std::vector<vk::DescriptorSet> shadowSets;
    Image *shadow;
    UniformShadow uShadow = {};
    std::vector<Buffer> shadowBuffers;

    std::vector<vk::DescriptorSet> sunSets;
    Image *sun;
    UniformSun uSun = {};
    std::vector<Buffer> sunBuffers;

    void create();
    void createColorSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createShadowSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createSunSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createUniformBuffers();

  private:
};

} // namespace tat