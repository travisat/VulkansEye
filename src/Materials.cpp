#include "Materials.hpp"
#include "Config.hpp"
#include <memory>
#include <type_traits>

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
    collection[0] = std::make_shared<Material>();
    int32_t index = 1;
    for (const auto &materialConfig : config.materials)
    {
        collection[index] = std::make_shared<Material>();
        // insert name into map for index retrieval
        names.insert(std::make_pair(materialConfig.name, index));
        // insert config into configs so material can be loaded when needed
        configs[index] = materialConfig;
        ++index;
    }
    debugLogger->info("Created Materials");
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
    auto material = collection[index];
    // if already loaded return
    if (material->loaded == true)
    {
        return;
    }

    // otherwise load the material
    material->name = configs[index].name;
    material->diffuse = loadImage(configs[index].diffuse);
    material->normal = loadImage(configs[index].normal);
    material->metallic = loadImage(configs[index].metallic);
    material->roughness = loadImage(configs[index].roughness);
    material->ao = loadImage(configs[index].ao);

    material->loaded = true;
}

auto Materials::loadImage(const std::string &path) -> std::shared_ptr<Image>
{
    auto image = std::make_shared<Image>(vulkan);
    image->imageInfo.usage =
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    image->memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    image->load(path);

    image->createSampler();
    return image;
}

} // namespace tat