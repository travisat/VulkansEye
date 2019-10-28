#include "Engine.hpp"
#include "Overlay.hpp"
#include "Timer.h"

namespace tat
{

enum class DisplayMode
{
    cursor = 0,
    nocursor = 1
};

class VulkansEye
{
  public:
    void init();
    void run();

  private:
    tat::Vulkan vulkan;
    Engine engine;
    Scene scene;
    Player player;
    Overlay overlay;

    DisplayMode displayMode = DisplayMode::nocursor;

    void cleanup();
    void mainLoop();
    void handleInput();

    static void framebufferResizeCallback(GLFWwindow *window, int /*width*/, int /*height*/)
    {
        //auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    };

    static void loadConfig(const std::string &path, Config &config);
};

} // namespace tat