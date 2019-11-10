#include "Backdrops.hpp"
#include "Config.hpp"
#include "Camera.hpp"
#include "Engine.hpp"
#include "Input.hpp"
#include "Materials.hpp"
#include "Meshes.hpp"
#include "Overlay.hpp"
#include "Timer.hpp"
#include <memory>

namespace tat
{

class VulkansEye
{
  public:
    explicit VulkansEye(const std::string &configPath);
    ~VulkansEye() = default;
    void run();

  private:
    std::shared_ptr<spdlog::logger> debugLogger;

    std::shared_ptr<Vulkan> vulkan = std::make_shared<Vulkan>();
    Engine engine{};
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Player> player;
    std::shared_ptr<Overlay> overlay;
    std::shared_ptr<Backdrops> backdrops;
    std::shared_ptr<Materials> materials;
    std::shared_ptr<Meshes> meshes;
    std::shared_ptr<Scene> scene;

    DisplayMode displayMode = DisplayMode::nocursor;

    void handleInput(float deltaTime);
};

} // namespace tat