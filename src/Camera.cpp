#include "Camera.hpp"
#include "Input.hpp"

namespace tat
{

Camera::Camera(const std::shared_ptr<Vulkan> &vulkan)
{
    this->vulkan = vulkan;
    width = static_cast<float>(vulkan->width);
    height = static_cast<float>(vulkan->height);
    setPosition(m_position);
    setRotation(m_rotation);
    updateView();
    updateProjection();
    spdlog::get("debugLogger")->info("Created Camera");
}

void Camera::look(double mouseX, double mouseY)
{
    // convert from glfw coordinates [0, width],[0, height] to [-1,1] interval
    mouseX = (mouseX / (width / 2)) - 1.F;
    mouseY = (mouseY / (height / 2)) - 1.F;
    glm::vec2 mousePosition(mouseX, mouseY);

    // discard old lastMousePosition so mouse doesn't jump when leaving insert
    if (Input::getMode() == InputMode::Insert)
    {
        lastMousePosition = mousePosition;
    }
    else if (lastMousePosition != mousePosition)
    { // don't update rotation  unless mouse is moved
        glm::vec2 deltaMousePosition = mousePosition - lastMousePosition;
        m_rotation.x -= deltaMousePosition.y * vulkan->mouseSensitivity;
        m_rotation.x = std::clamp(m_rotation.x, -90.0F, 90.0F);
        m_rotation.y += deltaMousePosition.x * vulkan->mouseSensitivity;
        if (m_rotation.y > 360.F)
        {
            m_rotation.y -= 360.F;
        }
        else if (m_rotation.y < 0.F)
        {
            m_rotation.y += 360.F;
        }
        lastMousePosition = mousePosition;
    }
}

void Camera::update()
{
    T = glm::translate(glm::mat4(1.F), m_position);
    R = glm::rotate(glm::mat4(1.F), glm::radians(m_rotation.x), glm::vec3(1.F, 0.F, 0.F));
    R = glm::rotate(R, glm::radians(m_rotation.y), glm::vec3(0.F, 1.F, 0.F));
    R = glm::rotate(R, glm::radians(m_rotation.z), glm::vec3(0.F, 0.F, 1.F));
    updateView();
}

auto Camera::position() -> glm::vec3
{
    return m_position;
}

auto Camera::rotation() -> glm::vec3
{
    return m_rotation;
}

void Camera::setPosition(glm::vec3 position)
{
    m_position = position;
}

void Camera::setRotation(glm::vec3 rotation)
{
    m_rotation = rotation;
}

void Camera::updateView()
{
    V = R * T;
}

void Camera::updateProjection()
{
    P = glm::perspective(glm::radians(vulkan->FoV), width / height, vulkan->zNear, vulkan->zFar);
}

auto Camera::view() -> glm::mat4
{
    return V;
}

auto Camera::projection() -> glm::mat4
{
    return P;
}

} // namespace tat