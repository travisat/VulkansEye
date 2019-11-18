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

VulkansEye::VulkansEye(const std::string &configPath)
{
    // init state
    auto &state = State::instance();

    // start timers
    Timer::getInstance();
    Timer::time();
    Timer::systemTime();
    spdlog::info("Started Timers");

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

    // setup input
    Input::getInstance();
    state.window.setKeyCallBack(&Input::keyCallback);
    state.window.setMouseButtonCallback(&Input::mouseButtonCallback);
    state.window.setCursorPosCallback(&Input::cursorPosCallback);
    state.window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    state.window.setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    spdlog::info("Created Input");

    // create engine
    state.engine.create();

    // create collections
    state.backdrops.create("backdrops");
    spdlog::info("Created Collection backdrops");
    state.materials.create("materials");
    spdlog::info("Created Collection materials");
    state.meshes.create("meshes");
    spdlog::info("Created Collection meshes");
    state.models.create("models");
    spdlog::info("Created Collection models");

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
    spdlog::info("Begin Main Loop");
    float lastFrameTime = 0.0F;
    while (state.window.shouldClose() == 0)
    {
        float now = Timer::time();
        float deltaTime = now - lastFrameTime;
        lastFrameTime = now;

        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2(state.window.getFrameBufferSize().first, state.window.getFrameBufferSize().second);
        io.DeltaTime = deltaTime;
        state.overlay.newFrame();
        state.overlay.updateBuffers();

        glfwPollEvents();
        handleInput(deltaTime);
        state.player.update(deltaTime);
        state.camera.setPosition(glm::vec3(-1.F, -1.F, -1.F) * state.player.position());
        state.camera.update();

        state.engine.drawFrame(deltaTime);
    }

    state.engine.device.waitIdle();
    spdlog::info("End Main Loop");
}

void VulkansEye::cleanup()
{
    auto &state = State::instance();
    state.backdrops.destroy();
    state.materials.destroy();
    state.meshes.destroy();
    state.models.destroy();

    tat::Camera::destroy();
    tat::Player::destroy();

    state.overlay.destroy();
    state.scene.destroy();
    state.engine.destroy();

    // dump state
    spdlog::get("state")->info(State::instance().dump(4));
}

void VulkansEye::handleInput(float deltaTime)
{
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

    if (Input::wasKeyReleased(GLFW_KEY_ESCAPE))
    {
        close();
    }
}

void VulkansEye::switchToNormalMode()
{
    auto &state = State::instance();
    if (Input::getMode() != InputMode::Normal)
    {
        // if mode is insert set glfw to take over cursor before switching
        if (Input::getMode() == InputMode::Insert)
        {
            state.window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            state.window.setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        state.engine.showOverlay = false;
        state.engine.updateCommandBuffer = true;
        Input::switchMode(InputMode::Normal);
        spdlog::info("Changed Mode to Normal");
    }
}

void VulkansEye::switchToVisualMode()
{
    auto &state = State::instance();

    if (Input::getMode() != InputMode::Visual)
    {
        // if mode is insert set glfw to take over cursor before switching
        if (Input::getMode() == InputMode::Insert)
        {
            state.window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            state.window.setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        state.engine.showOverlay = true;
        state.engine.updateCommandBuffer = true;
        Input::switchMode(InputMode::Visual);
        spdlog::info("Changed Mode to Visual");
    }
}

void VulkansEye::switchToInsertMode()
{
    auto &state = State::instance();
    if (Input::getMode() != InputMode::Insert)
    {
        // all other modes glfw owns cursor
        // switch it back
        state.window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        state.engine.showOverlay = true;
        state.engine.updateCommandBuffer = true;
        Input::switchMode(InputMode::Insert);
        spdlog::info("Changed Mode to Insert");
    }
}

void VulkansEye::close()
{
    auto &state = State::instance();
    spdlog::info("Closing");
    state.window.setClose(1);
}

} // namespace tat