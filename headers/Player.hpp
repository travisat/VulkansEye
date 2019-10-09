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

class Player
{
public:
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
   
    glm::mat4 perspective;
    glm::mat4 view;

    glm::vec3 force = glm::vec3(0.0f);

    
    float height = 1.75;//m
    float mass = 100;//kg

    //force applied while walking
    float velocityMax = 5.0f; //m/s  also people in games don't walk normal up this a bit
    float timeToReachVMax = 0.6f; //s
    float timeToStopfromVMax = 0.1f; //s
    

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
    float mouseSensitivity = 20.0f;
   
    void rotate(glm::vec3 delta);
    void translate(glm::vec3 delta);

};