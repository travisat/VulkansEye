#pragma once
#include <memory>
#include <utility>
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Vulkan.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "Player.hpp"
#include "Overlay.hpp"
#include "Scene.hpp"
#include "Collection.hpp"

namespace tat
{

class State: public json
{
  public:
    State(State const &) = delete;
    State(State&&) = delete;
    void operator=(State const &) = delete;
    void operator=(State&&) = delete;

    static auto instance() -> State &
    {
        static State instance;
        return instance;
    };
    
    std::shared_ptr<Vulkan> vulkan;
    std::shared_ptr<Window> window;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Player> player;
    std::shared_ptr<Overlay> overlay;
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Collection<Backdrop>> backdrops;
    std::shared_ptr<Collection<Material>> materials;
    std::shared_ptr<Collection<Mesh>> meshes;
    std::shared_ptr<Collection<Model>> models;

  private:
    State() = default;
};

} // namespace tat