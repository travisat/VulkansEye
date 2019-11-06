#include "Backdrops.hpp"
#include "Config.hpp"
#include "Engine.hpp"
#include "Materials.hpp"
#include "Meshes.hpp"
#include "Overlay.hpp"
#include "Timer.hpp"

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
    void init(const std::string &configPath);
    void run();

  private:
    Vulkan vulkan{};
    Engine engine{};
    Player player{};
    Overlay overlay{};
    Backdrops backdrops{};
    Materials materials{};
    Meshes meshes{};
    Scene scene{};

    DisplayMode displayMode = DisplayMode::nocursor;

    void cleanup();
    void mainLoop();
    void handleInput();

    static void framebufferResizeCallback(GLFWwindow *window, int /*width*/, int /*height*/){
        // auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    };

    static auto createConfig(const std::string &path) -> Config;
    static auto createPlayerconfig(const std::string &path) -> PlayerConfig;
    static auto createMaterialsConfig(const std::string &path) -> MaterialsConfig;
    static auto createMeshesConfig(const std::string &path) -> MeshesConfig;
    static auto createBackdropsConfig(const std::string &path) -> BackdropsConfig;
    static auto createSceneConfig(const std::string &path) -> SceneConfig;
};

} // namespace tat