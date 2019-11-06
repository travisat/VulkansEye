#pragma once

#include "Backdrop.hpp"
#include "Config.hpp"

namespace tat
{

class Backdrops
{
  public:
    Vulkan *vulkan;
    Player *player;

    void loadConfig(const BackdropsConfig &config);
    auto getBackdrop(const std::string &name) -> Backdrop *;

  private:
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