#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Config.hpp"
#include "Vulkan.hpp"
#include "Image.hpp"

namespace tat
{

struct Material
{
    std::string name = "";
    bool loaded = false;

    Image diffuse {};
    Image normal {};
    Image metallic {};
    Image roughness {};
    Image ao {};
    Image displacement {};    
};

class Materials
{
  public:
    Vulkan *vulkan = nullptr;

    //copies Material configs into config vector
    void loadConfig(const MaterialsConfig &config);
    
    //return pointer to material with name
    //loads material if it has not been loaded yet
    auto getMaterial(const std::string& name) -> Material *;

  private:
    //vector of configs, use loadConfigs to populate
    std::vector<MaterialConfig> configs {};
    //string index = material index
    std::map<std::string, int32_t> names {};
    //vector of empty Materials until material has been loaded 
    std::vector<Material> collection {};

    //loads material at index
    void loadMaterial(int32_t index);
    void loadImage(const std::string &path, Image &image);
    //returns meterial index of name
    //return 0 if name not found
    //0 is default material
    auto getIndex(const std::string& name) -> int32_t;
};

} // namespace tat