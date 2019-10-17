#include "Player.hpp"
#include "helpers.h"

namespace tat
{

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

void Player::rotate(glm::vec3 delta)
{
    this->rotation += delta;
    updateView();
}

void Player::translate(glm::vec3 delta)
{
    this->position += delta;
    updateView();
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
        rotation.y += deltaMousePosition.x * mouseSensitivity;
        lastMousePosition = mousePosition;
    }

    glm::vec3 camFront;
    camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
    camFront.y = sin(glm::radians(rotation.x));
    camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
    camFront = glm::normalize(camFront);

    float jumpingForce = 0.0f;

    // get rotational vector for direction to move
    glm::vec3 direction = glm::vec3(0.0f);
    if (Input::isKeyPressed(GLFW_KEY_W))
    {
        direction += camFront * glm::vec3(1.0f, 0.0f, 1.0f); // forward with way player looking
    }
    if (Input::isKeyPressed(GLFW_KEY_S))
    {
        direction -= camFront * glm::vec3(1.0f, 0.0f, 1.0f); // backward .
    }
    if (Input::isKeyPressed(GLFW_KEY_A))
    {
        direction -= glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f)); // left .
    }
    if (Input::isKeyPressed(GLFW_KEY_D))
    {
        direction += glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f)); // right .
    }
    if (Input::isKeyPressed(GLFW_KEY_SPACE))
    {
        if (position.y == -height)
        {
            jumpingForce = -jForce;
        }
    }
    // if we have a direction to move
    force = glm::vec3(0.0f);
    if (direction != glm::vec3(0.0f))
    {
        direction = normalize(direction);
        // turn rotational vector into force for walking
        float walkingForce = mass * (velocityMax / timeToReachVMax);

        force = direction * walkingForce;

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
        float stoppingForce = mass * (velocityMax / timeToStopfromVMax);
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

    force.y = jumpingForce;

    acceleration = force / mass;
    if (position.y < -height)
    {
        acceleration.y += 9.8f;
    }
    // apply acceleration to velocity
    velocity += deltaTime * acceleration;
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