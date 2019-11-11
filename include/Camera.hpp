#pragma once

#include "Vulkan.hpp"

namespace tat
{

class Camera
{
  public:
    explicit Camera(const std::shared_ptr<Vulkan> &vulkan);
    ~Camera() = default;

    bool mouseMode = true;

    void look(double mouseX, double mouseY);
    void update();

    auto position() -> glm::vec3;
    auto rotation() -> glm::vec3;
    void setPosition(glm::vec3 position);
    void setRotation(glm::vec3 rotation);

    void updateView();
    void updateProjection();
    auto view() -> glm::mat4;
    auto projection() -> glm::mat4;

  private:
    std::shared_ptr<Vulkan> vulkan;
    glm::mat4 T{}; // translation matrix
    glm::mat4 R{}; // rotation matrix

    glm::mat4 V{}; // view matrix
    glm::mat4 P{}; // projection matrix

    glm::vec3 m_position{};
    glm::vec3 m_rotation{};

    float width;
    float height;
    glm::vec2 lastMousePosition = glm::vec2(0.F);
};

} // namespace tat