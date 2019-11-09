#pragma once

#include "Config.hpp"
#include "Object.hpp"
#include "Vulkan.hpp"

namespace tat
{

class Player : public Object
{
  public:
    explicit Player(const std::shared_ptr<Vulkan> &vulkan, const PlayerConfig &config);
    ~Player() = default;

    void look(double mouseX, double mouseY);
    void move(glm::vec2 direction, float deltaTime);
    void jump();

    void updateAspectRatio(float windowWidth, float windowHeight);

    bool mouseMode = true;

    auto height() -> float
    {
        return scale().y;
    };

  private:
    float fieldOfView;
    float zNear;
    float zFar;

    float windowWidth;
    float windowHeight;

    float jumpVelocity = 0.F; // sqrt(2.0 * Gravity * heightofJump)

    // force applied while walking
    float velocityMax;        // m/s
    float timeToReachVMax;    // s
    float timeToStopfromVMax; // s

    float mouseSensitivity = 33.4F;

    glm::vec2 lastMousePosition = glm::vec2(0.F);

    auto onGround() -> bool;
};

} // namespace tat