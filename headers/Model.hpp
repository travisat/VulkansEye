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
    std::vector<Buffer> vertexBuffers;
    std::vector<Buffer> uniformLights;

    Image *irradianceMap;
    Image *radianceMap;
    Image *brdf;

    // shadow pipeline
    std::vector<vk::DescriptorSet> shadowSets;
    Image *shadow;
    std::vector<Buffer> shadowBuffers;

    std::vector<vk::DescriptorSet> sunSets;
    Image *sun;
    std::vector<Buffer> sunBuffers;

    void create();
    void createColorSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createShadowSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createSunSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createUniformBuffers();

  private:
};

} // namespace tat