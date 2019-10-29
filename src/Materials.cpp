#include "Materials.hpp"

namespace tat
{
void Materials::loadConfigs(const std::vector<MaterialConfig> &materials)
{
    
    // resize and allow for 0 index to be default
    configs.resize(materials.size() + 1);
    collection.resize(configs.size());
    int32_t index = 1; // start at 1 because index of 0 is reserved for default
    for (const auto& materialConfig : materials)
    {
        collection[index].name = materialConfig.name;
        //insert name into map for index retrieval
        names.insert(std::make_pair(materialConfig.name, index));
        //insert config into configs so material can be loaded when needed
        configs[index] = materialConfig;
        ++index;
    }
}

auto Materials::getMaterial(const std::string &name) -> Material *
{
    //get index, returns 0 for default if name of material not found
    int32_t index = getIndex(name);
    //load material if not loaded
    loadMaterial(index);
    //return material
    return &collection[index];
}

void Materials::loadMaterial(int32_t index)
{
    // get pointer to material
    Material *material = &collection[index];
    // if already loaded return
    if (material->loaded == true)
    {
        return;
    }
    // otherwise load the material
    loadImage(configs[index].diffuse, material->diffuse);
    loadImage(configs[index].normal, material->normal);
    loadImage(configs[index].metallic, material->metallic);
    loadImage(configs[index].roughness, material->roughness);
    loadImage(configs[index].ao, material->ao);
    loadImage(configs[index].displacement, material->displacement);

    material->loaded = true;
}

void Materials::loadImage(const std::string &path, Image &image)
{
    image.vulkan = vulkan;
    image.imageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    image.loadSTB(path);

    image.createSampler();
}

auto Materials::getIndex(const std::string &name) -> int32_t
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