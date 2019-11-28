#include "engine/Window.hpp"
#include "State.hpp"

#include <spdlog/spdlog.h>
#include <stdexcept>

namespace tat
{

void Window::create(void *user, int width, int height, const std::string &name)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, user);
    this->width = width;
    this->height = height;

    if constexpr (Debug::enable)
    {
        spdlog::info("Created window");
    }
}

void Window::destroy()
{
    glfwDestroyWindow(window);
    glfwTerminate();

    if constexpr (Debug::enable)
    {
        spdlog::info("Destroyed Window");
    }
}

void Window::resize()
{
    auto &state = State::instance();

    width = 0;
    height = 0;
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        wait();
    }

    state.engine.device.wait();
    state.at("settings").at("window").at(0) = width;
    state.at("settings").at("window").at(1) = height;
}

void Window::setKeyCallBack(GLFWkeyfun callback)
{
    glfwSetKeyCallback(window, callback);
}

void Window::setMouseButtonCallback(GLFWmousebuttonfun callback)
{
    glfwSetMouseButtonCallback(window, callback);
}

void Window::setCursorPosCallback(GLFWcursorposfun callback)
{
    glfwSetCursorPosCallback(window, callback);
}

void Window::setInputMode(int mode, int value)
{
    glfwSetInputMode(window, mode, value);
}

auto Window::shouldClose() -> int
{
    return glfwWindowShouldClose(window);
}

void Window::setClose(int value)
{
    glfwSetWindowShouldClose(window, value);
}

auto Window::createSurface(vk::Instance &instance) -> vk::SurfaceKHR
{
    vk::SurfaceKHR surface{};
    auto result = glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&surface));
    if (result != VK_SUCCESS)
    {
        spdlog::error("Unable to create surface. Error code {}", result);
        throw std::runtime_error("Unable to create surface");
    }
    return surface;
}

} // namespace tat