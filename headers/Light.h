#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Config.h"


class Light
{
public:
Light(LightConfig const &config);

glm::vec3 color {};
glm::vec3 position {};
glm::vec3 rotation {};


private:

uint32_t id = 0;

};