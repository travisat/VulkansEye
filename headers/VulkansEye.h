#include "VkBackend.h"

class VulkansEye
{
public:
    VulkansEye();
    void init(uint32_t width, uint32_t height);
    void run();

private:
    GLFWwindow *window;
    uint32_t windowWidth;
    uint32_t windowHeight;
    VkBackend *backend;
    
    void initWindow();
    void initVulkan();

    void drawFrame();

    void setupInputCallbacks();
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void mainLoop();
    void cleanup();

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto app = reinterpret_cast<VkBackend *>(glfwGetWindowUserPointer(window));
        framebufferResized = true;
    };
};