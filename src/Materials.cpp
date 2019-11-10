#include "Materials.hpp"
#include "Config.hpp"

namespace tat
{

Materials::Materials(const std::shared_ptr<Vulkan> &vulkan, const std::string &configPath)
{
    debugLogger = spdlog::get("debugLogger");
    this->vulkan = vulkan;
    auto config = MaterialsConfig(configPath);
    // resize and allow for 0 index to be default
    configs.resize(config.materials.size() + 1);
    collection.resize(configs.size() + 1);
    int32_t index = 1;
    for (const auto &materialConfig : config.materials)
    {
        collection[index].name = materialConfig.name;
        // insert name into map for index retrieval
        names.insert(std::make_pair(materialConfig.name, index));
        // insert config into configs so material can be loaded when needed
        configs[index] = materialConfig;
        ++index;
    }
    debugLogger->info("Loaded Materials");
}

auto Materials::getIndex(const std::string &name) -> int32_t
{
    auto result = names.find(name);
    int32_t index = 0;
    if (result != names.end())
    {
       index = result->second;
    }
    loadMaterial(index);
    return index;
}

void Materials::loadMaterial(int32_t index)
{
    // get pointer to material
    Material *material = &collection[index];
    // if already loaded return
    if (material->loaded == true)
    {
        return;
    }
    // otherwise load the material
    loadImage(configs[index].diffuse, material->diffuse);
    loadImage(configs[index].normal, material->normal);
    loadImage(configs[index].metallic, material->metallic);
    loadImage(configs[index].roughness, material->roughness);
    loadImage(configs[index].ao, material->ao);
    loadImage(configs[index].displacement, material->displacement);

    material->loaded = true;
}

void Materials::loadImage(const std::string &path, Image &image)
{
    image.vulkan = vulkan;
    image.imageUsage =
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    image.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    image.loadSTB(path);

    image.createSampler();
}

} // namespace tat