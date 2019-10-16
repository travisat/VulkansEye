#include <glm/gtc/constants.hpp>

#include "Overlay.hpp"
#include "Timer.h"
#include "VkEngine.hpp"


namespace tat {

class VulkansEye {
public:
  void init(uint32_t width, uint32_t height);
  void run();

private:
  tat::Vulkan vulkan;
  VkEngine engine;
  Scene scene;
  Player player;
  Overlay overlay;

  void drawFrame();

  void mainLoop();
  void cleanup();

  static void framebufferResizeCallback(GLFWwindow *window, int width,
                                        int height) {
    auto app = reinterpret_cast<VkEngine *>(glfwGetWindowUserPointer(window));
    framebufferResized = true;
  };
};

} // namespace tat