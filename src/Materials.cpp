#include "Materials.hpp"
#include "Config.hpp"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <memory>
#include <type_traits>

namespace tat
{

Material::Material(const std::shared_ptr<Vulkan> &vulkan, const std::shared_ptr<MaterialConfig> &config)
{
    auto logger = spdlog::get("debugLogger");
    this->vulkan = vulkan;
    name = config->name;
    path = config->path;
    diffuse = loadImage(config->diffuse);
    normal = loadImage(config->normal);
    metallic = loadImage(config->metallic);
    roughness = loadImage(config->roughness);
    ao = loadImage(config->ao);
    loaded = true;
    logger->info("Loaded Material {}", name);
}

auto Material::loadImage(const std::string &name) -> std::shared_ptr<Image>
{
    auto image = std::make_shared<Image>(vulkan);
    image->imageInfo.usage =
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    image->memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    image->load(path + "/" + name);

    image->createSampler();
    return image;
}

Materials::Materials(const std::shared_ptr<Vulkan> &vulkan, const std::string &path)
{
    debugLogger = spdlog::get("debugLogger");
    this->vulkan = vulkan;

    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        configs.push_back(std::make_shared<MaterialConfig>(entry.path().string()));
    }

    int32_t index = 0;
    for (const auto &materialConfig : configs)
    {
        collection.push_back(std::make_shared<Material>());
        // insert name into map for index retrieval
        names.insert(std::make_pair(materialConfig->name, index));
        // insert config into configs so material can be loaded when needed
        configs[index] = materialConfig;
        debugLogger->info("Materials added Config for {} to collection at index {}", materialConfig->name, index);
        ++index;
    }
    debugLogger->info("Created Materials");
}

auto Materials::getMaterial(const std::string &name) -> std::shared_ptr<Material>
{
    auto result = names.find(name);
    int32_t index = 0;
    if (result != names.end())
    {
        index = result->second;
    }
    if (collection[index]->loaded == false)
    {
        collection[index] = std::make_shared<Material>(vulkan, configs[index]);
    }
    return collection[index];
}

} // namespace tat