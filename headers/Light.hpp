#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Config.h"


struct UniformLightObject
{
    glm::vec4 lights[2];
    float exposure = 4.5f;
    float gamma = 2.2f;
};

class Light
{
public:
    Light(LightConfig const &config);

    void load();

    glm::vec4 light;
    float lumens = 0;
    float temperature = 0;

private:
    uint32_t id = 0;
};