#include "Light.h"


Light::Light(LightConfig const &config)
{
    this->id = config.id;
    this->position = config.position;
    this->rotation = config.rotation;
    this->color = config.color;
}