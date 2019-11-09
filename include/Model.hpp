#pragma once

#include <cassert>
#include <memory>
#include <unordered_map>

#include "Buffer.hpp"
#include "Config.hpp"
#include "Image.hpp"
#include "Materials.hpp"
#include "Meshes.hpp"
#include "Object.hpp"
#include "Timer.hpp"
#include "Vertex.hpp"

namespace tat
{

class Model : public Object
{
  public:
    // config values
    std::shared_ptr<Vulkan> vulkan;
    ModelConfig config;
    std::shared_ptr<Materials> materials;
    std::shared_ptr<Meshes> meshes;

    std::string name;

    const Image *irradianceMap;
    const Image *radianceMap;
    const Image *brdf;
    const Image *shadow;

    std::vector<vk::DescriptorSet> colorSets;
    std::vector<Buffer> vertBuffers;
    std::vector<Buffer> fragBuffers;
    std::vector<vk::DescriptorSet> shadowSets;
    std::vector<Buffer> shadBuffers;

    void create();
    void createColorSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createShadowSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createUniformBuffers();

    inline auto getMesh() -> Mesh *
    {
        return meshes->getMesh(meshIndex);
    };

  private:
    int32_t meshIndex;
    int32_t materialIndex;
};

} // namespace tat