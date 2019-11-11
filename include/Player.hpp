#pragma once

#include "Camera.hpp"
#include "Config.hpp"
#include "Object.hpp"

namespace tat
{

class Player : public Object
{
  public:
    explicit Player(const std::shared_ptr<Camera> &camera, const std::string &configPath);
    ~Player() = default;

    void move(glm::vec2 direction, float deltaTime);
    void jump();

    auto height() -> float
    {
        return scale().y;
    };

  private:
    std::shared_ptr<Camera> camera;
    std::shared_ptr<spdlog::logger> debugLogger;

    float jumpVelocity = 0.F; // sqrt(2.0 * Gravity * heightofJump)
    // force applied while walking
    float velocityMax;        // m/s
    float timeToReachVMax;    // s
    float timeToStopfromVMax; // s

    auto onGround() -> bool;
};

} // namespace tat