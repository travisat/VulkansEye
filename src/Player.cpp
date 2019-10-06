#include "Player.hpp"
#include "helpers.h"

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
    //convert to vulkan
    perspective = clip * perspective;
}

void Player::updateAspectRatio(double width, double height)
{
    this->windowWidth = width;
    this->windowHeight = height;
    perspective = glm::perspective(glm::radians(fieldOfView), (width / height), zNear, zFar);
    //convert to vulkan
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

void Player::update(double deltaTime)
{
    bool updated = true;
    if (Input::checkKeyboard(GLFW_KEY_A) | Input::checkKeyboard(GLFW_KEY_S) | Input::checkKeyboard(GLFW_KEY_D) | Input::checkKeyboard(GLFW_KEY_W))
    {
        updated = false;
    }
    double mouseX = Input::getMouseX();
    double mouseY = Input::getMouseY();

    //convert from glfw coordinates [0, width],[0, height] to [-1,1] interval
    mouseX = (mouseX / (windowWidth / 2)) - 1.0f;
    mouseY = (mouseY / (windowHeight / 2)) - 1.0f;

    glm::vec2 mousePosition(mouseX, mouseY);

    //discard old lastMousePosition so mouse doesn't jump when entering mouse mode
    if (mouseMode == false)
    {
        mouseMode = true;
        lastMousePosition = mousePosition;
    }

    if (lastMousePosition != mousePosition)
    {
        glm::vec2 deltaPosition = mousePosition - lastMousePosition;

        rotation.x += deltaPosition.y * mouseSensitivity;
        rotation.y += deltaPosition.x * mouseSensitivity;

        lastMousePosition = mousePosition;
        updated = false;
    }

    if (!updated)
    {
        glm::vec3 camFront;
        camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
        camFront.y = sin(glm::radians(rotation.x));
        camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
        camFront = glm::normalize(camFront);

        float moveSpeed = (float)(deltaTime * movementSpeed);

        if (Input::checkKeyboard(GLFW_KEY_W))
            position += camFront * glm::vec3(1.0f, 0.0f, 1.0f) * moveSpeed;
        if (Input::checkKeyboard(GLFW_KEY_S))
            position -= camFront * glm::vec3(1.0f, 0.0f, 1.0f) * moveSpeed;
        if (Input::checkKeyboard(GLFW_KEY_A))
            position -= glm::cross(camFront, (glm::vec3(0.0f, 1.0f, 0.0f) * moveSpeed));
        if (Input::checkKeyboard(GLFW_KEY_D))
            position += glm::cross(camFront, (glm::vec3(0.0f, 1.0f, 0.0f) * moveSpeed));
        if (Input::checkKeyboard(GLFW_KEY_SPACE))
            position -= glm::vec3(0.0f, 1.0f, 0.0f) * moveSpeed;
        if (Input::checkKeyboard(GLFW_KEY_LEFT_SHIFT))
            position += glm::vec3(0.0f, 1.0f, 0.0f) * moveSpeed;

        updateView();
    }
}
