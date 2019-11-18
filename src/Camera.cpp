#include "Camera.hpp"
#include "Input.hpp"
#include "State.hpp"

#include <spdlog/spdlog.h>

namespace tat
{

void Camera::create()
{
    auto &settings = State::instance().at("settings");
    width = settings.at("window").at(0);
    height = settings.at("window").at(1);
    FoV = settings.at("FoV");
    zNear = settings.at("zNear");
    zFar = settings.at("zFar");
    mouseSensitivity = settings.at("mouseSensitivity");
    setPosition(m_position);
    setRotation(m_rotation);
    updateView();
    updateProjection();
    currentMode = Input::getMode();
    spdlog::info("Created Camera");
}

void Camera::destroy()
{
    spdlog::info("Destroyed Camera");
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
        m_rotation.x -= deltaMousePosition.y * mouseSensitivity;
        m_rotation.x = std::clamp(m_rotation.x, -90.0F, 90.0F);
        m_rotation.y += deltaMousePosition.x * mouseSensitivity;
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
    P = glm::perspective(glm::radians(FoV), width / height, zNear, zFar);
}

void Camera::updateProjection(std::pair<int, int> size)
{
    width = static_cast<float>(size.first);
    height = static_cast<float>(size.second);
    updateProjection();
}

auto Camera::view() -> glm::mat4
{
    return V;
}

auto Camera::projection() -> glm::mat4
{
    return P;
}

void Camera::updateFov(float FoV)
{
    this->FoV = FoV;
    auto &settings = State::instance().at("settings");
    settings["FoV"] = FoV;
    updateProjection();
}

void Camera::updateZNear(float zNear)
{
    this->zNear = zNear;
    auto &settings = State::instance().at("settings");
    settings["zNear"] = zNear;
    updateProjection();
}

void Camera::updateZFar(float zFar)
{
    this->zFar = zFar;
    auto &settings = State::instance().at("settings");
    settings["zFar"] = zFar;
    updateProjection();
}

} // namespace tat