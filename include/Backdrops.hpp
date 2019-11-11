#pragma once
#include <map>

#include "Backdrop.hpp"
#include "Config.hpp"

namespace tat
{

class Backdrops
{
  public:
    Backdrops(const std::shared_ptr<Vulkan> &vulkan, const std::shared_ptr<Camera> &camera,
              const std::string &configPath);
    ~Backdrops() = default;

    std::shared_ptr<Vulkan> vulkan;
    std::shared_ptr<Camera> camera;

    auto getBackdrop(const std::string &name) -> Backdrop *;

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
    // vector of configs, use loadConfigs to populate
    std::vector<BackdropConfig> configs{};
    // vector of empty Backdrops until mesh has been loaded
    std::vector<Backdrop> collection{};
    // string index = mesh index
    std::map<std::string, int32_t> names{};

    auto getIndex(const std::string &name) -> int32_t;
    void loadBackdrop(int32_t index);
};

} // namespace tat