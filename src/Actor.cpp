#include "Actor.hpp"

#include "helpers.h"

namespace tat
{

void Actor::create()
{
    // TODO(travisat): implement using path to load model
    name = config->name;

    model.vulkan = vulkan;
    model.config = &config->model;
    model.shadow = shadow;
    model.materials = materials;
    model.create();
}

} // namespace tat