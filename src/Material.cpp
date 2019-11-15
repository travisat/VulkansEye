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
    auto& material = State::instance().at("materials").at(name);
    diffuse = loadImage(material.at("diffuse"));
    normal = loadImage(material.at("normal"));
    metallic = loadImage(material.at("metallic"));
    roughness = loadImage(material.at("roughness"));
    ao = loadImage(material.at("ao"));
    loaded = true;
    spdlog::info("Loaded Material {}", name);
}

auto Material::loadImage(const std::string &file) -> std::shared_ptr<Image>
{
    auto path = State::instance().at("settings").at("materialsPath").get<std::string>();
    auto image = std::make_shared<Image>();
    image->imageInfo.usage =
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    image->memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    image->load(path + name + "/" + file);

    image->createSampler();
    return image;
}

} // namespace tat