#include "PointLight.hpp"

void PointLight::load()
{
    name = config->name;
    light.position = config->position;
    light.temperature = config->temperature;
    light.lumens = config->lumens;
}