#pragma once

#include <algorithm>

#include "Config.h"

#include "Input.hpp"
#include "Vulkan.hpp"

namespace tat
{

const glm::mat4 clip(1.0f, 0.0f, 0.0f, 0.0f,  //
                     0.0f, -1.0f, 0.0f, 0.0f, //
                     0.0f, 0.0f, 0.5f, 0.0f,  //
                     0.0f, 0.0f, 0.5f, 1.0f); //

class Player
{
  public:
    Vulkan *vulkan;
    PlayerConfig *config;

    void create();

    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
    glm::vec3 force = glm::vec3(0.0f);
    glm::vec3 moveDir = glm::vec3(0.0f);

    glm::mat4 perspective;
    glm::mat4 view;

    float height;
    float mass; // kg
    float jumpVelocity = 3.0; //sqrt(2.0 * Gravity * heightofJump) jump of half .45m
    float lastTime = 0.0f;

    // force applied while walking
    float velocityMax;        // m/s  also people in games don't walk normal up this a
                              // bit
    float timeToReachVMax;    // s
    float timeToStopfromVMax; // s

    float mouseSensitivity = 33.4f;

    void update(float deltaTime);
    void move(glm::vec2 direction);
    void jump();

    void applyForce(glm::vec3 force);

    void updateView();

    void setPerspective(double fieldOfView, double windowWidth, double windowHeight, double zNear, double zFar);
    void updateAspectRatio(double windowWidth, double windowHeight);
    double getNearClip()
    {
        return zNear;
    };
    double getFarClip()
    {
        return zFar;
    };

    bool mouseMode = true;

  private:
    double fieldOfView;
    double zNear, zFar;
    double windowWidth;
    double windowHeight;

    

    glm::vec2 lastMousePosition = glm::vec2(0.0f);

    void rotate(glm::vec3 delta);
    void translate(glm::vec3 delta);
};

} // namespace tat