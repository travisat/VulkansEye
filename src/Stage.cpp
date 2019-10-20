#include "Stage.hpp"
#include "helpers.h"

namespace tat
{

void Stage::create()
{
    backdrop.vulkan = vulkan;
    backdrop.player = player;
    backdrop.path = config->backdrop;
    backdrop.create();

    models.resize(config->models.size());
    for (auto &model : config->models)
    {
        models[model.index].config = &model;
        models[model.index].vulkan = vulkan;
        models[model.index].create();
    }
}

void Stage::createDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorLayout)
{
    for (auto &model : models)
    {
        model.createDescriptorSets(descriptorPool, descriptorLayout);
    }
}

} // namespace tat