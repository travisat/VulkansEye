#include "PointLight.hpp"

namespace tat
{

void PointLight::create()
{
    name = config->name;
    light.position = config->position;
    light.color = kelvinToRGB(config->temperature);
    light.lumens = config->lumens;

    model.vulkan = vulkan;
    model.config = &config->model;
    model.create();
}

// https://www.shadertoy.com/view/lsSXW1
// http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
// http://www.zombieprototypes.com/?p=210
// converts light temperurate in kelvin to RGB
glm::vec3 PointLight::kelvinToRGB(float kelvin)
{
    glm::vec3 color;
    kelvin = glm::clamp(kelvin, 1000.0f, 40000.0f) / 100.0f;
    if (kelvin <= 66.0)
    {
        color.r = 1.0;
        color.g = glm::clamp(-0.606464f - 0.001742f * (kelvin - 2.0f) + 0.408173f * log(kelvin - 2.0f), 0.0f, 1.0f);
        if (kelvin <= 20.0)
            color.b = 0.0;
        else
            color.b = glm::clamp(-0.995193f + 0.000323f * (kelvin - 10.0f) + 0.451875f * log(kelvin - 10.0f), 0.0f, 1.0f);
    }
    else
    {
        color.r = glm::clamp(1.374910f - 0.001742f * (kelvin - 55.0f) + 0.408173f * log(kelvin - 55.0f), 0.0f, 1.0f);
        color.g = glm::clamp(1.271287f - 0.0003f * (kelvin - 50.0f) - 0.109708f * log(kelvin - 50.0f), 0.0f, 1.0f);
        color.b = 1.0;
    }
    return color;
}

} // namespace tat