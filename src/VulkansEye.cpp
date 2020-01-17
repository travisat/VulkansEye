#include "VulkansEye.hpp"
#include "Config.hpp"
#include "Input.hpp"
#include "State.hpp"
#include "Timer.hpp"

#include <exception>
#include <memory>

#include <spdlog/spdlog.h>

namespace tat
{

// window resize callback
static void resizeWindow(GLFWwindow * /*window*/, int width, int height)
{
    State::instance().engine.resize(width, height);
};

VulkansEye::VulkansEye(const std::string &configPath)
{
    // init state
    auto &state = State::instance();

    // start timers
    Timer::getInstance();
    Timer::time();
    if constexpr (Debug::enable)
    {
        spdlog::info("Started Timers");
    }

    // init mode stack
    Input::pushMode(InputMode::Normal);

    // load config
    Config config;
    config.create(configPath);

    // get settings
    auto &settings = state.at("settings");

    // load display settings
    if (settings.at("vsync").get<bool>())
    {
        state.engine.defaultPresentMode = vk::PresentModeKHR::eFifo;
    }
    else
    {
        state.engine.defaultPresentMode = vk::PresentModeKHR::eMailbox;
    }

    // load glfw window
    auto &window = settings.at("window");
    state.window.create(this, window.at(0), window.at(1), "Vulkans Eye");
    state.window.setWindowSizeCallBack(&resizeWindow);

    // setup input
    Input::getInstance();
    state.window.setKeyCallBack(&Input::keyCallback);
    state.window.setMouseButtonCallback(&Input::mouseButtonCallback);
    state.window.setCursorPosCallback(&Input::cursorPosCallback);
    state.window.setCharCallback(&Input::charCallback);
    state.window.setScrollCallback(&Input::scrollCallback);
    state.window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    state.window.setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    if constexpr (Debug::enable)
    {
        spdlog::info("Created Input");
    }

    // create engine
    state.engine.create();

    // create collections
    state.backdrops.create("backdrops");
    if constexpr (Debug::enable)
    {
        spdlog::info("Created Collection backdrops");
    }
    state.materials.create("materials");
    if constexpr (Debug::enable)
    {
        spdlog::info("Created Collection materials");
    }
    state.meshes.create("meshes");
    if constexpr (Debug::enable)
    {
        spdlog::info("Created Collection meshes");
    }
    state.models.create("models");
    if constexpr (Debug::enable)
    {
        spdlog::info("Created Collection models");
    }

    // create other objects
    state.player.create();
    state.camera.create();
    state.scene.create();
    state.overlay.create();

    // prepare engine
    state.engine.prepare();
}

void VulkansEye::run()
{
    auto &state = State::instance();
    if constexpr (Debug::enable)
    {
        spdlog::info("Begin Main Loop");
    }

    for (auto lastFrameTime = 0.F; state.window.shouldClose() == 0;)
    {
        auto now = Timer::time();
        auto deltaTime = now - lastFrameTime;
        lastFrameTime = now;

        handleInput(deltaTime);

        state.player.update(deltaTime);
        state.camera.setPosition(-1.F * state.player.position());
        state.camera.update();
        state.overlay.update(deltaTime);
        state.engine.drawFrame(deltaTime);
    }

    state.engine.device.wait();
    if constexpr (Debug::enable)
    {
        spdlog::info("End Main Loop");
    }
}

void VulkansEye::cleanup()
{
    auto &state = State::instance();

    Camera::destroy();
    Player::destroy();

    state.overlay.destroy();
    state.scene.destroy();

    state.backdrops.destroy();
    state.materials.destroy();
    state.meshes.destroy();
    state.models.destroy();

    state.engine.destroy();

    // dump state
    if constexpr (Debug::enable)
    {
        spdlog::get("state")->info(State::instance().dump(4));
    }
}

void VulkansEye::handleInput(float deltaTime)
{
    glfwPollEvents();

    auto &state = State::instance();
    // Normal Mode
    if (Input::wasKeyReleased(GLFW_KEY_F1))
    {
        switchToNormalMode();
    }

    // Visual Mode
    if (Input::wasKeyReleased(GLFW_KEY_F2))
    {
        switchToVisualMode();
    }

    // Insert Mode
    if (Input::wasKeyReleased(GLFW_KEY_F3))
    {
        switchToInsertMode();
    }

    // Paused Mode
    if (Input::wasKeyReleased(GLFW_KEY_ESCAPE))
    {
        switchToPausedMode();
    }

    state.camera.look(Input::getMouseX(), Input::getMouseY());

    auto moveDir = glm::vec2(0.F);
    if (Input::getMode() != InputMode::Insert)
    { // don't move camera/character in insert mode

        moveDir.y -= static_cast<float>(Input::isKeyPressed(GLFW_KEY_W));
        moveDir.y += static_cast<float>(Input::isKeyPressed(GLFW_KEY_S));
        moveDir.x += static_cast<float>(Input::isKeyPressed(GLFW_KEY_A));
        moveDir.x -= static_cast<float>(Input::isKeyPressed(GLFW_KEY_D));

        if (Input::isKeyPressed(GLFW_KEY_SPACE) != 0)
        {
            state.player.jump();
        }
    }
    // still update player for friction
    state.player.move(moveDir, deltaTime);
}

void VulkansEye::switchToNormalMode()
{
    auto &state = State::instance();
    state.window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    state.window.setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    state.overlay.settings.showEditor = false;
    state.overlay.settings.showInfo = false;
    state.overlay.settings.showPaused = false;

    state.engine.showOverlay = false;
    state.engine.updateCommandBuffer = true;
    Input::popMode();
    Input::pushMode(InputMode::Normal);

    if constexpr (Debug::enable)
    {
        spdlog::info("Changed Mode to Normal");
    }
}

void VulkansEye::switchToVisualMode()
{
    auto &state = State::instance();
    state.window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    state.overlay.settings.showEditor = false;
    state.overlay.settings.showInfo = true;
    state.overlay.settings.showPaused = false;

    state.engine.showOverlay = true;
    Input::popMode();
    Input::pushMode(InputMode::Visual);

    if constexpr (Debug::enable)
    {
        spdlog::info("Changed Mode to Visual");
    }
}

void VulkansEye::switchToInsertMode()
{
    auto &state = State::instance();
    state.window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    state.overlay.settings.showEditor = true;
    state.overlay.settings.showInfo = false;
    state.overlay.settings.showPaused = false;

    state.engine.showOverlay = true;
    Input::popMode();
    Input::pushMode(InputMode::Insert);

    if constexpr (Debug::enable)
    {
        spdlog::info("Changed Mode to Insert");
    }
}

void VulkansEye::switchToPausedMode()
{
    // only pause on normal mode
    if (Input::getMode() == InputMode::Normal)
    {
        auto &state = State::instance();
        state.window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        state.overlay.settings.showEditor = false;
        state.overlay.settings.showInfo = false;
        state.overlay.settings.showPaused = true;

        state.engine.showOverlay = true;
        Input::pushMode(InputMode::Paused);

        if constexpr (Debug::enable)
        {
            spdlog::info("Changed Mode to Paused");
        }
        return;
    }

    if (Input::getMode() == InputMode::Insert)
    {
        auto &editor = State::instance().overlay.editor;
        if (editor.getMode() != Zep::EditorMode::Normal)
        {
            // don't do anything with escape if editor is not in normal mode
            return;
        }
    }

    // otherwise go back to previous mode
    Input::popMode();
    switch (Input::getMode())
    {
    case InputMode::Normal:
        switchToNormalMode();
        break;

    case InputMode::Visual:
        switchToVisualMode();
        break;

    case InputMode::Insert:
        switchToInsertMode();
        break;

    default:;
    }
}

} // namespace tat