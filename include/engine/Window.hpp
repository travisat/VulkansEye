#pragma once
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <utility>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{

class Window
{
  public:
    void create(void *user, int width, int height, const std::string &name);
    void destroy();

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
