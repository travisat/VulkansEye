#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Input.hpp"

const glm::mat4 clip(1.0f, 0.0f, 0.0f, 0.0f,
                     0.0f, -1.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 0.5f, 0.0f,
                     0.0f, 0.0f, 0.5f, 1.0f);

class Camera
{
public:
    glm::vec3 rotation = glm::vec3();
    glm::vec3 position = glm::vec3();

    glm::mat4 perspective;
    glm::mat4 view;

    double getNearClip() { return zNear; };
    double getFarClip() { return zFar; };

    void updateView();

    void setPerspective(double fieldOfView, double width, double height, double zNear, double zFar);
    void updateAspectRatio(double width, double height);

    void rotate(glm::vec3 delta);
    void translate(glm::vec3 delta);

    void update(float deltaTime);

    bool mouseMode = false;

private:
    double fieldOfView;
    double zNear, zFar;
    double width;
    double height;

    glm::vec2 lastMousePosition = glm::vec2(0.0f, 0.0f);

    float mouseSensitivity = 20.0f;
    float movementSpeed = 0.01f;
};