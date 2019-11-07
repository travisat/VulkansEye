#include "Backdrops.hpp"
#include "Config.hpp"
#include "Engine.hpp"
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
    std::shared_ptr<Vulkan> vulkan = std::make_shared<Vulkan>();
    Engine engine{};
    std::shared_ptr<Player> player = std::make_shared<Player>();
    std::shared_ptr<Overlay> overlay = std::make_shared<Overlay>();
    std::shared_ptr<Backdrops> backdrops = std::make_shared<Backdrops>();
    std::shared_ptr<Materials> materials = std::make_shared<Materials>();
    std::shared_ptr<Meshes> meshes = std::make_shared<Meshes>();
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();

    DisplayMode displayMode = DisplayMode::nocursor;

    void handleInput();

    static auto createConfig(const std::string &path) -> Config;
    static auto createPlayerconfig(const std::string &path) -> PlayerConfig;
    static auto createMaterialsConfig(const std::string &path) -> MaterialsConfig;
    static auto createMeshesConfig(const std::string &path) -> MeshesConfig;
    static auto createBackdropsConfig(const std::string &path) -> BackdropsConfig;
    static auto createSceneConfig(const std::string &path) -> SceneConfig;
};

} // namespace tat