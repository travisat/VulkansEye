#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Input.hpp"

namespace tat
{

const glm::mat4 clip(1.0f, 0.0f, 0.0f, 0.0f,
                     0.0f, -1.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 0.5f, 0.0f,
                     0.0f, 0.0f, 0.5f, 1.0f);

class Player
{
public:
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
    glm::vec3 force = glm::vec3(0.0f);

    glm::mat4 perspective;
    glm::mat4 view;

    float height;
    float mass;   //kg
    float jForce;;

    //force applied while walking
    float velocityMax;        //m/s  also people in games don't walk normal up this a bit
    float timeToReachVMax;    //s
    float timeToStopfromVMax; //s

    float mouseSensitivity = 33.4f;

    void update(float deltaTime);

    void applyForce(glm::vec3 force);

    void updateView();

    void setPerspective(double fieldOfView, double windowWidth, double windowHeight, double zNear, double zFar);
    void updateAspectRatio(double windowWidth, double windowHeight);
    double getNearClip() { return zNear; };
    double getFarClip() { return zFar; };

    bool mouseMode = false;

private:
    double fieldOfView;
    double zNear, zFar;
    double windowWidth;
    double windowHeight;

    glm::vec2 lastMousePosition = glm::vec2(0.0f, 0.0f);

    void rotate(glm::vec3 delta);
    void translate(glm::vec3 delta);
};

} //namespace tat