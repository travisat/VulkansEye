#include "Light.hpp"


Light::Light(LightConfig const &config)
{
    this->id = config.id;
    this->light = config.light;
    this->temperature = config.temperature;
    this->lumens = config.lumens;
}