#include "VulkansEye.hpp"
#include "Config.hpp"
#include "Input.hpp"
#include "State.hpp"
#include "Timer.hpp"
#include "spdlog/spdlog.h"
#include <exception>
#include <memory>

namespace tat
{

VulkansEye::VulkansEye(const std::string &configPath)
{
    // init state
    auto &state = State::instance();
    state.vulkan = std::make_shared<Vulkan>();

    // start timers
    Timer::getInstance();
    Timer::time();
    Timer::systemTime();
    spdlog::info("Started Timers");

    // load config
    auto config = Config(configPath);

    //get settings
    auto &settings = state.at("settings");

    // load display settings
    if (settings.at("vsync").get<bool>() == true)
    {
        state.vulkan->defaultPresentMode = vk::PresentModeKHR::eFifo;
    }
    else
    {
        state.vulkan->defaultPresentMode = vk::PresentModeKHR::eMailbox;
    }

    // setup glfw window
    auto& window = settings.at("window");
    state.window = std::make_shared<Window>(this, window.at(0), window.at(1), "Vulkans Eye");

    // setup input
    Input::getInstance();
    state.window->setKeyCallBack(&Input::keyCallback);
    state.window->setMouseButtonCallback(&Input::mouseButtonCallback);
    state.window->setCursorPosCallback(&Input::cursorPosCallback);
    state.window->setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    state.window->setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    spdlog::info("Created Input");

    // init engine
    engine.init();

    // init collections
   
    state.backdrops = std::make_shared<Collection<Backdrop>>("backdrops");
    spdlog::info("Created Collection backdrops");
    state.materials = std::make_shared<Collection<Material>>("materials");
    spdlog::info("Created Collection materials");
    state.meshes = std::make_shared<Collection<Mesh>>("meshes");
    spdlog::info("Created Collection meshes");
    state.models = std::make_shared<Collection<Model>>("models");
    spdlog::info("Created Collection models");
    

    //init other objects
    state.camera = std::make_shared<Camera>();
    state.player = std::make_shared<Player>();
    state.scene = std::make_shared<Scene>();
    state.scene->load();
    state.overlay = std::make_shared<Overlay>();

    // prepare engine
    engine.prepare();
}

void VulkansEye::run()
{
    auto& state = State::instance();
    spdlog::info("Begin Main Loop");
    float lastFrameTime = 0.0F;
    while (state.window->shouldClose() == 0)
    {
        float now = Timer::time();
        float deltaTime = now - lastFrameTime;
        lastFrameTime = now;

        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2(state.window->getFrameBufferSize().first, state.window->getFrameBufferSize().second);
        io.DeltaTime = deltaTime;
        state.overlay->newFrame();
        state.overlay->updateBuffers();

        glfwPollEvents();
        handleInput(deltaTime);
        state.player->update(deltaTime);
        state.camera->setPosition(glm::vec3(-1.F, -1.F, -1.F) * state.player->position());
        state.camera->update();

        engine.drawFrame(deltaTime);
    }
    vkDeviceWaitIdle(state.vulkan->device);
    spdlog::info("End Main Loop");

    //dump state
    spdlog::get("state")->info(State::instance().dump(4));
}

void VulkansEye::handleInput(float deltaTime)
{
    auto& state = State::instance();
    // Normal Mode
    if (Input::wasKeyReleased(GLFW_KEY_F1))
    {
        if (Input::getMode() != InputMode::Normal)
        {
            // if mode is insert set glfw to take over cursor before switching
            if (Input::getMode() == InputMode::Insert)
            {
                state.window->setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                state.window->setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }

            engine.showOverlay = false;
            engine.updateCommandBuffer = true;
            Input::switchMode(InputMode::Normal);
            spdlog::info("Changed Mode to Normal");
        }
    }
    // Visual Mode
    if (Input::wasKeyReleased(GLFW_KEY_F2))
    {
        if (Input::getMode() != InputMode::Visual)
        {
            // if mode is insert set glfw to take over cursor before switching
            if (Input::getMode() == InputMode::Insert)
            {
                state.window->setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                state.window->setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }

            engine.showOverlay = true;
            engine.updateCommandBuffer = true;
            Input::switchMode(InputMode::Visual);
            spdlog::info("Changed Mode to Visual");
        }
    }

    // Insert Mode
    if (Input::wasKeyReleased(GLFW_KEY_F3))
    {
        if (Input::getMode() != InputMode::Insert)
        {
            // all other modes glfw owns cursor
            // switch it back
            state.window->setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            engine.showOverlay = true;
            engine.updateCommandBuffer = true;
            Input::switchMode(InputMode::Insert);
            spdlog::info("Changed Mode to Insert");
        }
    }

    auto moveDir = glm::vec2(0.F);
    if (Input::getMode() != InputMode::Insert)
    { // don't move camera/character in insert mode
        state.camera->look(Input::getMouseX(), Input::getMouseY());

        moveDir.y -= static_cast<float>(Input::isKeyPressed(GLFW_KEY_W));
        moveDir.y += static_cast<float>(Input::isKeyPressed(GLFW_KEY_S));
        moveDir.x += static_cast<float>(Input::isKeyPressed(GLFW_KEY_A));
        moveDir.x -= static_cast<float>(Input::isKeyPressed(GLFW_KEY_D));

        if (Input::isKeyPressed(GLFW_KEY_SPACE) != 0)
        {
            state.player->jump();
        }
    }
    // still update player for friction
    state.player->move(moveDir, deltaTime);

    if (Input::wasKeyReleased(GLFW_KEY_ESCAPE))
    {
        spdlog::info("Pressed Escape Closing");
        state.window->setClose(1);
    }
}

} // namespace tat