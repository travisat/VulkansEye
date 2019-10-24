#include "Actor.hpp"

#include "helpers.h"

namespace tat
{

void Actor::create()
{
    // TODO implement using path to load model
    name = config->name;

    model.vulkan = vulkan;
    model.config = &config->model;
    model.shadow = shadow;
    model.shadowSampler = shadowSampler;
    model.create();
}

} // namespace tat