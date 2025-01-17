#pragma once

#include "Object.hpp"

namespace tat
{

class Player : public Object
{
  public:
    void create();
    static void destroy();

    void move(glm::vec2 direction, float deltaTime);
    void jump();

    auto height() -> float
    {
        return scale().y;
    };

  private:
    float jumpVelocity = 0.F; // sqrt(2.0 * Gravity * heightofJump)
    // force applied while walking
    float velocityMax{};        // m/s
    float timeToReachVMax{};    // s
    float timeToStopfromVMax{}; // s

    auto onGround() -> bool;
};

} // namespace tat