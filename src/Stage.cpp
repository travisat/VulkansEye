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
        models[model.index].shadow = shadow;
        models[model.index].shadowSampler = shadowSampler;
        models[model.index].create();
    }
}

void Stage::createColorSets(VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
    for (auto &model : models)
    {
        model.createColorSets(pool, layout);
    }
}

void Stage::createShadowSets(VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
    for (auto &model : models)
    {
        model.createShadowSets(pool, layout);
    }
}

} // namespace tat