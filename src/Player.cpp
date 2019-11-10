#include "Player.hpp"
#include "Config.hpp"
#include "glm/gtx/string_cast.hpp"
#include "helpers.hpp"

namespace tat
{

Player::Player(const std::shared_ptr<Vulkan> &vulkan, const std::string &configPath)
{
    debugLogger = spdlog::get("debugLogger");
    auto config = PlayerConfig(configPath);
    auto size = glm::vec3(0.5F, config.height * 2.F, 0.25F);
    translate(config.position + size / 2.F);
    rotate(config.rotation);
    m_size = size;
    m_mass = config.mass;

    fieldOfView = config.fieldOfView;
    jumpVelocity = glm::sqrt(2.0F * 9.8F * config.jumpHeight);
    velocityMax = config.velocityMax;
    timeToReachVMax = config.timeToReachVMax;
    timeToStopfromVMax = config.timeToStopfromVMax;
    mouseSensitivity = config.mouseSensitivity;
    windowWidth = vulkan->width;
    windowHeight = vulkan->height;
    zNear = vulkan->zNear;
    zFar = vulkan->zFar;
    updateAspectRatio(windowWidth, windowHeight);
    updateView(R * T);
}

void Player::updateAspectRatio(float width, float height)
{
    this->windowWidth = width;
    this->windowHeight = height;
    updateProjection(glm::perspective(glm::radians(fieldOfView), (windowWidth / windowHeight), zNear, zFar));
}

void Player::move(glm::vec2 direction, float deltaTime)
{
    glm::vec3 camFront;
    camFront.x = -cos(glm::radians(m_rotation.x)) * sin(glm::radians(m_rotation.y));
    camFront.y = sin(glm::radians(m_rotation.x));
    camFront.z = cos(glm::radians(m_rotation.x)) * cos(glm::radians(m_rotation.y));
    camFront = glm::normalize(camFront);

    // convert input move direction into player view
    auto moveDir = glm::vec3(0.F);
    moveDir += direction.y * camFront * glm::vec3(1.F, 0.F, 1.F);
    moveDir += direction.x * glm::cross(camFront, glm::vec3(0.F, 1.F, 0.F));

    auto force = glm::vec3(0.F);
    if (glm::length(moveDir) > 0.F)
    { // we want to move
        float walking = (m_mass * velocityMax) / (timeToReachVMax * deltaTime);
        force = normalize(moveDir) * walking;

        if (glm::length(m_velocity) > 0.F)
        { // if we are moving apply friction
            float internalFriction = (glm::length(m_velocity) / velocityMax) * walking;
            force -= internalFriction * normalize(m_velocity);
        }
    }
    else if (onGround())
    { // we want to stop
        if (length(m_velocity) > 0.F)
        { // if stopped don't apply friction
            float stoppingForce = (m_mass * velocityMax) / (timeToStopfromVMax * deltaTime);
            float internalFriction = (glm::length(m_velocity) / velocityMax) * stoppingForce;
            force = internalFriction * (-1.F * normalize(m_velocity));
        }
    }

    applyForce(force);
}

void Player::jump()
{
    if (onGround())
    {
        m_velocity.y += jumpVelocity;
    }
}

void Player::look(double mouseX, double mouseY)
{
    // convert from glfw coordinates [0, width],[0, height] to [-1,1] interval
    mouseX = (mouseX / (windowWidth / 2)) - 1.F;
    mouseY = (mouseY / (windowHeight / 2)) - 1.F;
    glm::vec2 mousePosition(mouseX, mouseY);

    // discard old lastMousePosition so mouse doesn't jump changing modes
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
        rotate();
    }
}

auto Player::onGround() -> bool
{ // TODO (travis) create collision to detect this
    return position().y - m_size.y / 2.F <= 0;
}

} // namespace tat