#pragma once

#include "Buffer.hpp"
#include "Config.hpp"
#include "Image.hpp"
#include "Materials.hpp"
#include "Meshes.hpp"
#include "Object.hpp"

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

    std::shared_ptr<Image> irradianceMap;
    std::shared_ptr<Image> radianceMap;
    std::shared_ptr<Image> brdf;
    std::shared_ptr<Image> shadow;

    std::vector<vk::DescriptorSet> colorSets;
    std::vector<Buffer> vertBuffers;
    std::vector<Buffer> fragBuffers;
    std::vector<vk::DescriptorSet> shadowSets;
    std::vector<Buffer> shadBuffers;

    void create();
    void createColorSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createShadowSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createUniformBuffers();

    inline auto getMesh() -> std::shared_ptr<Mesh>
    {
        return meshes->getMesh(meshIndex);
    };

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
    int32_t meshIndex;
    int32_t materialIndex;
};

} // namespace tat