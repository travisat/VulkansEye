#include "Window.hpp"
#include "helpers.hpp"
#include "vulkan/vulkan.hpp"

namespace tat
{

Window::Window(void *user, int width, int height, const std::string &name)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, user);
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
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
    CheckResult(glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&surface)));
    return surface;
}

auto Window::getFrameBufferSize() -> std::pair<int, int>
{
    int width;
    int height;
    glfwGetFramebufferSize(window, &width, &height);
    return std::make_pair(width, height);
}

} // namespace tat