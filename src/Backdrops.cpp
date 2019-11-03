#include "Backdrops.hpp"

namespace tat
{
void Backdrops::loadConfig(const BackdropsConfig &config)
{
    
    // resize and allow for 0 index to be default
    configs.resize(config.backdrops.size() + 1);
    collection.resize(configs.size());
    int32_t index = 1; // start at 1 because index of 0 is reserved for default
    for (const auto& backdropConfig : config.backdrops)
    {
        collection[index].name = backdropConfig.name;
        names.insert(std::make_pair(backdropConfig.name, index));
        configs[index] = backdropConfig;
        ++index;
    }
}

auto Backdrops::getBackdrop(const std::string &name) -> Backdrop *
{
    //get index, returns 0 for default if name of material not found
    int32_t index = getIndex(name);
    //load material if not loaded
    loadBackdrop(index);
    //return material
    return &collection[index];
}

void Backdrops::loadBackdrop(int32_t index)
{
    // get pointer to material
    Backdrop *backdrop = &collection[index];
    // if already loaded return
    if (backdrop->loaded == true)
    {
        return;
    }
    // otherwise load the material
    backdrop->player = player;
    backdrop->vulkan = vulkan;
    backdrop->config = &configs[index];
    backdrop->create();
    backdrop->loaded = true;
}

auto Backdrops::getIndex(const std::string &name) -> int32_t
{
    auto result = names.find(name);
    if (result == names.end())
    {
        // if name not in list return 0 index which is for default material
        return 0;
    }
    // otherwise return index for name
    return result->second;
}

} // namespace tat