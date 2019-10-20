#include "Player.hpp"
#include "helpers.h"

namespace tat
{

void Player::create()
{
    setPerspective(config->fieldOfView, static_cast<double>(vulkan->width),
                          static_cast<double>(vulkan->height), 0.1f, 512.0f);
    position = config->position;
    rotation = config->rotation;
    height = config->height;
    mass = config->mass;
    jumpVelocity = glm::sqrt(2.0f * 9.8f * config->jumpHeight);
    velocityMax = config->velocityMax;
    timeToReachVMax = config->timeToReachVMax;
    timeToStopfromVMax = config->timeToStopfromVMax;
    mouseSensitivity = config->mouseSensitivity;
    updateView();
}

void Player::updateView()
{

    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    glm::mat4 translationMatrix;

    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    translationMatrix = glm::translate(glm::mat4(1.0f), position);

    view = rotationMatrix * translationMatrix;
}

void Player::setPerspective(double fieldOfView, double width, double height, double zNear, double zFar)
{
    this->fieldOfView = fieldOfView;
    this->windowWidth = width;
    this->windowHeight = height;
    this->zNear = zNear;
    this->zFar = zFar;
    perspective = glm::perspective(glm::radians(fieldOfView), width / height, zNear, zFar);
    // convert to vulkan
    perspective = clip * perspective;
}

void Player::updateAspectRatio(double width, double height)
{
    this->windowWidth = width;
    this->windowHeight = height;
    perspective = glm::perspective(glm::radians(fieldOfView), (width / height), zNear, zFar);
    // convert to vulkan
    perspective = clip * perspective;
}

void Player::move(glm::vec2 direction)
{
    glm::vec3 camFront;
    camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
    camFront.y = sin(glm::radians(rotation.x));
    camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
    camFront = glm::normalize(camFront);

    moveDir = glm::vec3(0.0f);
    moveDir += direction.y * camFront * glm::vec3(1.0f, 0.0f, 1.0f);
    moveDir += direction.x * glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Player::jump()
{
    if (position.y == -height)
    {
        velocity.y -= jumpVelocity; 
    }
}

void Player::update(float deltaTime)
{

    double mouseX = Input::getMouseX();
    double mouseY = Input::getMouseY();
    // convert from glfw coordinates [0, width],[0, height] to [-1,1] interval
    mouseX = (mouseX / (windowWidth / 2)) - 1.0f;
    mouseY = (mouseY / (windowHeight / 2)) - 1.0f;
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
        rotation.x += deltaMousePosition.y * mouseSensitivity;
        rotation.x = std::clamp(rotation.x, -90.0f, 90.0f);
        rotation.y += deltaMousePosition.x * mouseSensitivity;
        lastMousePosition = mousePosition;
    }

    force = glm::vec3(0.0f);
    if (moveDir != glm::vec3(0.0f))
    {
        moveDir = normalize(moveDir);
        // turn rotational vector into force for walking
        float walkingForce = (mass * velocityMax) / (timeToReachVMax * deltaTime);

        force = moveDir * walkingForce;

        if (glm::abs(velocity.x) < 0.001f && glm::abs(velocity.z) < 0.001) // if stopped don't apply friction
        {
            velocity.x = 0.0f;
            velocity.z = 0.0f;
        }
        else // apply friction
        {
            float internalFriction =
                (glm::length(velocity) / velocityMax) * walkingForce; // internal friction, our legs only move so fast
            force -= internalFriction * normalize(velocity);
        }
    }
    else if (position.y == -height)
    { // we want to stop
        float stoppingForce = (mass * velocityMax) / (timeToStopfromVMax * deltaTime);
        if (glm::abs(velocity.x) < 0.001f && glm::abs(velocity.z) < 0.001) // if stopped don't apply friction
        {
            velocity.x = 0.0f;
            velocity.z = 0.0f;
        }
        else // apply stopping force
        {
            float internalFriction = (glm::length(velocity) / velocityMax) * stoppingForce;
            force = internalFriction * (-1.0f * normalize(velocity));
        }
    }

    // set acceleration
    acceleration = deltaTime * force / mass;
    
    if (position.y < -height)
    {
        acceleration.y += 9.8f;
    }
    // apply acceleration to velocity
    velocity += acceleration * deltaTime;
    // apply velocity to position
    position += velocity * deltaTime;
    if (position.y > -height)
    {
        position.y = -height;
        velocity.y = 0.0f;
    }

    updateView();
}

} // namespace tat