#pragma once

#include <algorithm>

#include "Config.h"

#include "Input.hpp"
#include "Vulkan.hpp"

namespace tat
{


class Player
{
  public:
    Vulkan *vulkan;
    PlayerConfig *config;

    void create();

    glm::vec3 rotation = glm::vec3(0.0F);
    glm::vec3 position = glm::vec3(0.0F);
    glm::vec3 velocity = glm::vec3(0.0F);
    glm::vec3 acceleration = glm::vec3(0.0F);
    glm::vec3 force = glm::vec3(0.0F);

    float height; // m
    float mass;   // kg

    glm::mat4 perspective;
    glm::mat4 view;

    void update(float deltaTime);
    void move(glm::vec2 direction);
    void jump();

    void applyForce(glm::vec3 force);

    void updateView();

    void updateAspectRatio(float windowWidth, float windowHeight);

    bool mouseMode = true;

  private:
    float fieldOfView;

    float windowWidth;
    float windowHeight;

    float jumpVelocity = 0.0F; // sqrt(2.0 * Gravity * heightofJump)
    float lastTime = 0.0F;     // s

    // force applied while walking
    float velocityMax;        // m/s
    float timeToReachVMax;    // s
    float timeToStopfromVMax; // s

    float mouseSensitivity = 33.4F;

    glm::vec2 lastMousePosition = glm::vec2(0.0F);
    glm::vec3 moveDir = glm::vec3(0.0F);
};

} // namespace tat