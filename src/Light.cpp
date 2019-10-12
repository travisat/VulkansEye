#include "Light.hpp"
#include "helpers.h"

void Light::load()
{
    name = config->name;
    position = config->position;
    temperature = config->temperature;
    lumens = config->lumens;
}