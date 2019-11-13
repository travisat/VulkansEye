#pragma once
#include <map>

#include "Backdrop.hpp"
#include "Config.hpp"

namespace tat
{

class Backdrops
{
  public:
    Backdrops(const std::shared_ptr<Vulkan> &vulkan, const std::shared_ptr<Camera> &camera);
    ~Backdrops() = default;

    std::shared_ptr<Vulkan> vulkan;
    std::shared_ptr<Camera> camera;

    auto getBackdrop(const std::string &name) -> std::shared_ptr<Backdrop>;

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
    // vector of empty Backdrops until mesh has been loaded
    std::vector<std::shared_ptr<Backdrop>> collection{};
    // string index = mesh index
    std::map<std::string, int32_t> names{};

    auto getIndex(const std::string &name) -> int32_t;
    auto loadBackdrop(int32_t index)-> std::shared_ptr<Backdrop>;
};

} // namespace tat