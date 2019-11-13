#include "Backdrops.hpp"
#include "Config.hpp"
#include "spdlog/spdlog.h"
#include <memory>

namespace tat
{
Backdrops::Backdrops(const std::shared_ptr<Vulkan>& vulkan, const std::shared_ptr<Camera>& camera)
{
    debugLogger = spdlog::get("debugLogger");
    this->vulkan = vulkan;
    this->camera = camera;

    auto& state = State::instance();
    
    int32_t index = 0;
    collection.resize(0);
    for ( auto &[key, config] : state["backdrops"].items())
    {
        collection.push_back(std::make_shared<Backdrop>());
        collection[index]->name = key;
        names.insert(std::make_pair(key, index));
        ++index;
    }
    if (collection.empty())
    {   //load default in if no configs found
        collection.resize(1);
        debugLogger->warn("Backdrops not found, using default");
    }
    
    debugLogger->info("Created Backdrops");
}

auto Backdrops::getBackdrop(const std::string &name) -> std::shared_ptr<Backdrop>
{
    //get index, returns 0 for default if name of material not found
    int32_t index = getIndex(name);
    //load material if not loaded
    loadBackdrop(index);
    //return material
    return collection[index];
}

auto Backdrops::loadBackdrop(int32_t index)->std::shared_ptr<Backdrop>
{
    auto &backdrop = collection[index];
    // if already loaded return
    if (collection[index]->loaded == true)
    {
        return collection[index];
    }
    // otherwise load the material
    backdrop->vulkan = vulkan;
    backdrop->camera = camera;
    backdrop->load();
    return backdrop;
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