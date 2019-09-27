#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



class Light
{
public:
Light(){};
Light(glm::vec3 _color, glm::vec3 _position, glm::vec3 _rotation)
    :color(_color), position(_position), rotation(_rotation){};

glm::vec3 color {};
glm::vec3 position {};
glm::vec3 rotation {};


private:

};