#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Config.hpp"
#include "Image.hpp"
#include "Vulkan.hpp"

namespace tat
{

struct Material
{
    std::string name = "";
    bool loaded = false;

    Image diffuse{};
    Image normal{};
    Image metallic{};
    Image roughness{};
    Image ao{};
    Image displacement{};
};

class Materials
{
  public:
    std::shared_ptr<Vulkan> vulkan;

    // copies Material configs into config vector
    void loadConfig(const MaterialsConfig &config);

    // returns meterial index of name
    // return 0 if name not found
    // 0 is default material
    // loads material if not loaded yet
    auto getIndex(const std::string &name) -> int32_t;

    inline auto getMaterial(int32_t index) -> Material *
    {
        if (index < collection.size() && index > 0)
        {
            return &collection[index];
        }
        // if index out of bounds just return default material
        return &collection[0];
    };

  private:
    std::vector<Material> collection{};

    // vector of configs, use loadConfigs to populate
    std::vector<MaterialConfig> configs{};
    // string index = material index
    std::map<std::string, int32_t> names{};

    // loads material at index
    void loadMaterial(int32_t index);
    void loadImage(const std::string &path, Image &image);
};

} // namespace tat