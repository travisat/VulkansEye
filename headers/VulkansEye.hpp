#include "VkEngine.hpp"
#include "Overlay.hpp"
#include "Timer.h"

class VulkansEye
{
public:
    VulkansEye();
    void init(uint32_t width, uint32_t height);
    void run();

private:
    tat::Vulkan vulkan{};
    VkEngine engine{};
    Overlay imgui{}; //at base becuase should have access to all info to display it
    Scene scene {};

    GLFWwindow *window = nullptr;

    int width = 0;
    int height = 0;

    void initWindow();
    void initVulkan();

    void drawFrame();

    void mainLoop();
    void cleanup();

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto app = reinterpret_cast<VkEngine *>(glfwGetWindowUserPointer(window));
        framebufferResized = true;
    };
};