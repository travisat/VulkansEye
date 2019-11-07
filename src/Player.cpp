#include "Player.hpp"
#include "helpers.hpp"

namespace tat
{

void Player::loadConfig(const PlayerConfig &config)
{
    m_position = config.position;
    m_rotation = config.rotation;
    m_size = glm::vec3(0.5F, config.height, 0.25F);
    m_mass = config.mass;

    fieldOfView = config.fieldOfView;
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
    glm::mat4 translate = glm::translate(glm::mat4(1.0F), m_position);

    glm::mat4 rotate = glm::mat4(1.0F);
    rotate = glm::rotate(rotate, glm::radians(m_rotation.x), glm::vec3(1.0F, 0.0F, 0.0F));
    rotate = glm::rotate(rotate, glm::radians(m_rotation.y), glm::vec3(0.0F, 1.0F, 0.0F));
    rotate = glm::rotate(rotate, glm::radians(m_rotation.z), glm::vec3(0.0F, 0.0F, 1.0F));

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
    camFront.x = -cos(glm::radians(m_rotation.x)) * sin(glm::radians(m_rotation.y));
    camFront.y = sin(glm::radians(m_rotation.x));
    camFront.z = cos(glm::radians(m_rotation.x)) * cos(glm::radians(m_rotation.y));
    camFront = glm::normalize(camFront);

    // convert input move direction into player view
    moveDir = glm::vec3(0.0F);
    moveDir += direction.y * camFront * glm::vec3(1.0F, 0.0F, 1.0F);
    moveDir += direction.x * glm::cross(camFront, glm::vec3(0.0F, 1.0F, 0.0F));
}

void Player::jump()
{
    if (m_position.y == m_size.y)
    {
        m_velocity.y += jumpVelocity;
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
        m_rotation.x -= deltaMousePosition.y * mouseSensitivity;
        m_rotation.x = std::clamp(m_rotation.x, -90.0F, 90.0F);
        m_rotation.y += deltaMousePosition.x * mouseSensitivity;
        lastMousePosition = mousePosition;
    }

    m_force = glm::vec3(0.0F);
    if (moveDir != glm::vec3(0.0F))
    {
        moveDir = normalize(moveDir);
        // turn rotational vector into force for walking
        float walkingForce = (m_mass * velocityMax) / (timeToReachVMax * deltaTime);

        m_force = moveDir * walkingForce;

        if (glm::abs(m_velocity.x) < 0.001F && glm::abs(m_velocity.z) < 0.001) // if stopped don't apply friction
        {
            m_velocity.x = 0.0F;
            m_velocity.z = 0.0F;
        }
        else // apply friction
        {
            float internalFriction =
                (glm::length(m_velocity) / velocityMax) * walkingForce; // internal friction, our legs only move so fast
            m_force -= internalFriction * normalize(m_velocity);
        }
    }
    else if (m_position.y == m_size.y)
    { // we want to stop
        float stoppingForce = (m_mass * velocityMax) / (timeToStopfromVMax * deltaTime);
        if (glm::abs(m_velocity.x) < 0.001F && glm::abs(m_velocity.z) < 0.001) // if stopped don't apply friction
        {
            m_velocity.x = 0.0F;
            m_velocity.z = 0.0F;
        }
        else // apply stopping force
        {
            float internalFriction = (glm::length(m_velocity) / velocityMax) * stoppingForce;
            m_force = internalFriction * (-1.0F * normalize(m_velocity));
        }
    }


    // set acceleration
    m_acceleration = deltaTime * m_force / m_mass;

    if (m_position.y > m_size.y)
    {
        m_acceleration.y -= 9.8F;
    }
    // apply acceleration to velocity
    m_velocity += m_acceleration * deltaTime;
    // apply velocity to position
    m_position += m_velocity * deltaTime;
    if (m_position.y < m_size.y)
    {
        m_position.y = m_size.y;
        m_velocity.y = 0.0F;
    }

    updateView();
}

} // namespace tat