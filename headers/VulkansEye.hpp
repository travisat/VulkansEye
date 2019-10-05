#include "VkEngine.hpp"
#include "Overlay.hpp"
#include "Timer.h"

class VulkansEye
{
public:
    void init(uint32_t width, uint32_t height);
    void run();

private:
    tat::Vulkan vulkan;
    VkEngine engine;

    void drawFrame();

    void mainLoop();
    void cleanup();

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto app = reinterpret_cast<VkEngine *>(glfwGetWindowUserPointer(window));
        framebufferResized = true;
    };
};