#include "Material.hpp"
#include "State.hpp"

#include <filesystem>
#include <memory>
#include <type_traits>

#include <spdlog/spdlog.h>

namespace tat
{

Material::~Material()
{
    diffuse.destroy();
    normal.destroy();
    metallic.destroy();
    roughness.destroy();
    ao.destroy();
}

void Material::load()
{
    //get json for material
    auto& material = State::instance().at("materials").at(name);
    //load textures in json
    loadImage(material.at("diffuse"), &diffuse);
    loadImage(material.at("normal"), &normal);
    loadImage(material.at("metallic"), &metallic);
    loadImage(material.at("roughness"), &roughness);
    loadImage(material.at("ao"), &ao);
    scale = material.at("scale");
    //we are now loaded
    loaded = true;
    spdlog::info("Loaded Material {}", name);
}

void Material::loadImage(const std::string &file, Image *image)
{
    auto path = State::instance().at("settings").at("materialsPath").get<std::string>();
    image->imageInfo.usage =
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    image->memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    image->load(path + name + "/" + file);

    image->createSampler();
}

} // namespace tat