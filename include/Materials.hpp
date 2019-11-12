#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Config.hpp"
#include "Image.hpp"
#include "Vulkan.hpp"

namespace tat
{

class Material
{
  public:
    // this constructor loads the material
    explicit Material(const std::shared_ptr<Vulkan> &vulkan, const std::shared_ptr<MaterialConfig> &config);
    // this loads default values loaded should be false until loaded
    Material() = default;
    ~Material() = default;
    std::shared_ptr<Vulkan> vulkan = nullptr;

    std::string name = "";
    bool loaded = false;
    std::string path = "";

    std::shared_ptr<Image> diffuse = nullptr;
    std::shared_ptr<Image> normal = nullptr;
    std::shared_ptr<Image> metallic = nullptr;
    std::shared_ptr<Image> roughness = nullptr;
    std::shared_ptr<Image> ao = nullptr;

  private:
    auto loadImage(const std::string &file) -> std::shared_ptr<Image>;
};

class Materials
{
  public:
    Materials(const std::shared_ptr<Vulkan> &vulkan, const std::string &path);
    ~Materials() = default;

    std::shared_ptr<Vulkan> vulkan;

    auto getMaterial(const std::string &name) -> std::shared_ptr<Material>;

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
    std::vector<std::shared_ptr<Material>> collection{};

    // vector of configs, use loadConfigs to populate
    std::vector<std::shared_ptr<MaterialConfig>> configs;
    // string index = material index
    std::map<std::string, int32_t> names{};
};

} // namespace tat