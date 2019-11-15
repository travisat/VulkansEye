#include "Window.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"
#include <stdexcept>
#include <spdlog/spdlog.h>

namespace tat
{

Window::Window(void *user, int width, int height, const std::string &name)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, user);
    spdlog::info("Created window {} with dimensions {}x{}", name.c_str(), width, height);
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
    auto result = glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&surface));
    if (result != VK_SUCCESS)
    {
        spdlog::error("Unable to create surface. Error code {}", result);
        throw std::runtime_error("Unable to create surface");
        return nullptr;
    }
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