#pragma once

#define GLM_FORCE_RADIANS
#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace tat
{

class Camera
{
  public:
    Camera();
    ~Camera() = default;

    void look(double mouseX, double mouseY);
    void update();

    auto position() -> glm::vec3;
    auto rotation() -> glm::vec3;
    void setPosition(glm::vec3 position);
    void setRotation(glm::vec3 rotation);

    void updateView();
    void updateProjection();
    void updateProjection(float width, float height);
    auto view() -> glm::mat4;
    auto projection() -> glm::mat4;

    void updateFov(float FoV);
    void updateZNear(float zNear);
    void updateZFar(float zFar);

  private:
    glm::mat4 T{}; // translation matrix
    glm::mat4 R{}; // rotation matrix

    glm::mat4 V{}; // view matrix
    glm::mat4 P{}; // projection matrix

    glm::vec3 m_position{};
    glm::vec3 m_rotation{};

    float width;
    float height;
    float FoV;
    float zNear;
    float zFar;
    glm::vec2 lastMousePosition = glm::vec2(0.F);
    float mouseSensitivity;
};

} // namespace tat