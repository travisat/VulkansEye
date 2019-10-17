#include <glm/gtc/constants.hpp>

#include "Overlay.hpp"
#include "Timer.h"
#include "Engine.hpp"

namespace tat
{

class VulkansEye
{
  public:
    void init(uint32_t width, uint32_t height);
    void run();

  private:
    tat::Vulkan vulkan;
    Engine engine;
    Scene scene;
    Player player;
    Overlay overlay;

    void cleanup();
    void mainLoop();
    void handleInput();

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
        framebufferResized = true;
    };
};

} // namespace tat