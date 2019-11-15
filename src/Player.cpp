#include "Player.hpp"
#include "State.hpp"

namespace tat
{

Player::Player()
{
    auto &player = State::instance().at("player");
    auto size = glm::vec3(0.5F, player.at("height").get<float>() * 2.F, 0.25F);
    translate(size / 2.F);
    rotate();
    m_size = size;
    m_mass = player.at("mass");

    jumpVelocity = glm::sqrt(2.0F * 9.8F * player.at("jumpHeight").get<float>());
    velocityMax = player.at("velocityMax");
    timeToReachVMax = player.at("timeToReachVMax");
    timeToStopfromVMax = player.at("timeToStopFromVMax");
    spdlog::info("Created Player");
}

void Player::move(glm::vec2 direction, float deltaTime)
{
    auto &state = State::instance();
    glm::vec3 camFront;
    glm::vec3 rotation = state.camera->rotation();
    camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
    camFront.y = sin(glm::radians(rotation.x));
    camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
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
        m_velocity.y -= jumpVelocity;
    }
}

auto Player::onGround() -> bool
{ // TODO (travis) create collision to detect this
    return position().y + m_size.y / 2.F >= 0;
}

} // namespace tat