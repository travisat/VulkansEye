#include "Player.hpp"
#include "helpers.h"

namespace tat
{

void Player::loadConfig(const PlayerConfig &config)
{
    fieldOfView = config.fieldOfView;
    position = config.position;
    rotation = config.rotation;
    height = config.height;
    mass = config.mass;
    jumpVelocity = glm::sqrt(2.0F * 9.8F * config.jumpHeight);
    velocityMax = config.velocityMax;
    timeToReachVMax = config.timeToReachVMax;
    timeToStopfromVMax = config.timeToStopfromVMax;
    mouseSensitivity = config.mouseSensitivity;
    updateAspectRatio(static_cast<float>(vulkan->width), static_cast<float>(vulkan->height));
    updateView();
}

void Player::updateView()
{
     glm::mat4 translate = glm::translate(glm::mat4(1.0F), position);

    glm::mat4 rotate = glm::mat4(1.0F);
    rotate = glm::rotate(rotate, glm::radians(rotation.x), glm::vec3(1.0F, 0.0F, 0.0F));
    rotate = glm::rotate(rotate, glm::radians(rotation.y), glm::vec3(0.0F, 1.0F, 0.0F));
    rotate = glm::rotate(rotate, glm::radians(rotation.z), glm::vec3(0.0F, 0.0F, 1.0F));

    view = rotate * translate;
}

void Player::updateAspectRatio(float width, float height)
{
    this->windowWidth = width;
    this->windowHeight = height;
    perspective = glm::perspective(glm::radians(fieldOfView), (width / height), vulkan->zNear, vulkan->zFar);
}

void Player::move(glm::vec2 direction)
{
    glm::vec3 camFront;
    camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
    camFront.y = sin(glm::radians(rotation.x));
    camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
    camFront = glm::normalize(camFront);

    // convert input move direction into player view
    moveDir = glm::vec3(0.0F);
    moveDir += direction.y * camFront * glm::vec3(1.0F, 0.0F, 1.0F);
    moveDir += direction.x * glm::cross(camFront, glm::vec3(0.0F, 1.0F, 0.0F));
}

void Player::jump()
{
    if (position.y == height)
    {
        velocity.y += jumpVelocity;
    }
}

void Player::update(float deltaTime)
{

    double mouseX = Input::getMouseX();
    double mouseY = Input::getMouseY();
    // convert from glfw coordinates [0, width],[0, height] to [-1,1] interval
    mouseX = (mouseX / (windowWidth / 2)) - 1.0F;
    mouseY = (mouseY / (windowHeight / 2)) - 1.0F;
    glm::vec2 mousePosition(mouseX, mouseY);
    // discard old lastMousePosition so mouse doesn't jump when entering mouse
    // mode
    if (mouseMode == false)
    {
        lastMousePosition = mousePosition;
    }
    if (lastMousePosition != mousePosition)
    {
        glm::vec2 deltaMousePosition = mousePosition - lastMousePosition;
        rotation.x -= deltaMousePosition.y * mouseSensitivity;
        rotation.x = std::clamp(rotation.x, -90.0F, 90.0F);
        rotation.y += deltaMousePosition.x * mouseSensitivity;
        lastMousePosition = mousePosition;
    }

    force = glm::vec3(0.0F);
    if (moveDir != glm::vec3(0.0F))
    {
        moveDir = normalize(moveDir);
        // turn rotational vector into force for walking
        float walkingForce = (mass * velocityMax) / (timeToReachVMax * deltaTime);

        force = moveDir * walkingForce;

        if (glm::abs(velocity.x) < 0.001F && glm::abs(velocity.z) < 0.001) // if stopped don't apply friction
        {
            velocity.x = 0.0F;
            velocity.z = 0.0F;
        }
        else // apply friction
        {
            float internalFriction =
                (glm::length(velocity) / velocityMax) * walkingForce; // internal friction, our legs only move so fast
            force -= internalFriction * normalize(velocity);
        }
    }
    else if (position.y == height)
    { // we want to stop
        float stoppingForce = (mass * velocityMax) / (timeToStopfromVMax * deltaTime);
        if (glm::abs(velocity.x) < 0.001F && glm::abs(velocity.z) < 0.001) // if stopped don't apply friction
        {
            velocity.x = 0.0F;
            velocity.z = 0.0F;
        }
        else // apply stopping force
        {
            float internalFriction = (glm::length(velocity) / velocityMax) * stoppingForce;
            force = internalFriction * (-1.0F * normalize(velocity));
        }
    }

    // set acceleration
    acceleration = deltaTime * force / mass;

    if (position.y > height)
    {
        acceleration.y -= 9.8F;
    }
    // apply acceleration to velocity
    velocity += acceleration * deltaTime;
    // apply velocity to position
    position += velocity * deltaTime;
    if (position.y < height)
    {
        position.y = height;
        velocity.y = 0.0F;
    }

    updateView();
}

} // namespace tat