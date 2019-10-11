#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Config.h"

//#include "Actor.hpp"
//#include "Model.hpp"

static const int numLights = 2;

struct UniformShaderObject
{
    glm::vec3 position;
    glm::vec3 color;
    float lumens;
    float exposure = 4.5f;
    float gamma = 2.2f;
};

class Light 
{
public:
    LightConfig *config;

    uint32_t id = 0;
    std::string name = "Unknown light";
    glm::vec3 position;
    glm::vec3 color;
    float lumens = 0;
    float temperature = 0;

    void load();

private:
};