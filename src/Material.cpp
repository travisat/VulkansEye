#include "Material.hpp"
#include "Config.hpp"
#include "State.hpp"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <memory>
#include <type_traits>

namespace tat
{

void Material::load()
{
    auto logger = spdlog::get("debugLogger");
    auto& material = State::instance().at("materials").at(name);
    path = material["path"];
    diffuse = loadImage(material["diffuse"]);
    normal = loadImage(material["normal"]);
    metallic = loadImage(material["metallic"]);
    roughness = loadImage(material["roughness"]);
    ao = loadImage(material["roughness"]);
    loaded = true;
    logger->info("Loaded Material {}", name);
}

auto Material::loadImage(const std::string &file) -> std::shared_ptr<Image>
{
    auto image = std::make_shared<Image>();
    image->imageInfo.usage =
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    image->memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    image->load(path + "/" + file);

    image->createSampler();
    return image;
}

} // namespace tat