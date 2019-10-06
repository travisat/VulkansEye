#include "Light.hpp"
#include "helpers.h"

void Light::load()
{
    name = config->name;
    position = config->position;
    color = config->position;
}