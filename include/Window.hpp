#pragma once

#include <utility>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

namespace tat
{

class Window
{
  public:
    Window(void *user, int width, int height, const std::string &name);
    ~Window();
    GLFWwindow *window;

    void setKeyCallBack(GLFWkeyfun callback);
    void setMouseButtonCallback(GLFWmousebuttonfun callback);
    void setCursorPosCallback(GLFWcursorposfun callback);
    void setInputMode(int mode, int value);

    auto shouldClose() -> int;
    void setClose(int value);

    auto createSurface(vk::Instance &instance) -> vk::SurfaceKHR;
    auto getFrameBufferSize() -> std::pair<int, int>;

    static void wait()
    {
        glfwWaitEvents();
    };
};

} // namespace tat
