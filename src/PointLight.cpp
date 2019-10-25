#include "PointLight.hpp"

namespace tat
{

void PointLight::create()
{
    name = config->name;
    light.position = config->position;
    light.color = kelvinToRGB(config->temperature);
    light.lumens = config->lumens;
}

// https://www.shadertoy.com/view/lsSXW1
// http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
// http://www.zombieprototypes.com/?p=210
// converts light temperurate in kelvin to RGB
auto PointLight::kelvinToRGB(float kelvin) -> glm::vec3
{
    glm::vec3 color;
    kelvin = glm::clamp(kelvin, 1000.0F, 40000.0F) / 100.0F;
    if (kelvin <= 66.0)
    {
        color.r = 1.0;
        color.g = glm::clamp(-0.606464F - 0.001742F * (kelvin - 2.0F) + 0.408173F * log(kelvin - 2.0F), 0.0F, 1.0F);
        if (kelvin <= 20.0)
        {
            color.b = 0.0;
        }
        else
        {
            color.b =
                glm::clamp(-0.995193F + 0.000323F * (kelvin - 10.0F) + 0.451875F * log(kelvin - 10.0F), 0.0F, 1.0F);
        }
    }
    else
    {
        color.r = glm::clamp(1.374910F - 0.001742F * (kelvin - 55.0F) + 0.408173F * log(kelvin - 55.0F), 0.0F, 1.0F);
        color.g = glm::clamp(1.271287F - 0.0003F * (kelvin - 50.0F) - 0.109708F * log(kelvin - 50.0F), 0.0F, 1.0F);
        color.b = 1.0;
    }
    return color;
}

} // namespace tat