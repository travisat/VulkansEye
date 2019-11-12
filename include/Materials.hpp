#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "Config.hpp"
#include "Image.hpp"
#include "Vulkan.hpp"

namespace tat
{

struct Material
{
    std::string name = "";
    bool loaded = false;

    std::shared_ptr<Image> diffuse = nullptr;
    std::shared_ptr<Image> normal = nullptr;
    std::shared_ptr<Image> metallic = nullptr;
    std::shared_ptr<Image> roughness = nullptr;
    std::shared_ptr<Image> ao = nullptr;
};

class Materials
{
  public:
    Materials(const std::shared_ptr<Vulkan> &vulkan, const std::string &configPath);
    ~Materials() = default;

    std::shared_ptr<Vulkan> vulkan;

    // returns meterial index of name
    // return 0 if name not found
    // 0 is default material
    // loads material if not loaded yet
    auto getIndex(const std::string &name) -> int32_t;

    inline auto getMaterial(int32_t index) -> std::shared_ptr<Material>
    {
        if (index < collection.size() && index > 0)
        {
            return collection[index];
        }
        // if index out of bounds just return default material
        return collection[0];
    };

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
    std::vector<std::shared_ptr<Material>> collection{};

    // vector of configs, use loadConfigs to populate
    std::vector<MaterialConfig> configs{};
    // string index = material index
    std::map<std::string, int32_t> names{};

    // loads material at index
    void loadMaterial(int32_t index);
    auto loadImage(const std::string &path) -> std::shared_ptr<Image>;
};

} // namespace tat