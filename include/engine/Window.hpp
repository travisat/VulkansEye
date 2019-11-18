#pragma once

#include <utility>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

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

    void resize();

    GLFWwindow *window = nullptr;
    int32_t width {};
    int32_t height {};

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
