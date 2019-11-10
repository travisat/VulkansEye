#include "VulkansEye.hpp"
#include "Backdrops.hpp"
#include "Config.hpp"
#include "Input.hpp"
#include <memory>

namespace tat
{

VulkansEye::VulkansEye(const std::string &configPath)
{
    debugLogger = spdlog::get("debugLogger");
    // start timers
    Timer::getInstance();
    Timer::time();
    Timer::systemTime();
    debugLogger->info("Started Timers");

    // load config
    auto config = Config(configPath);

    // load display settings
    vulkan->name = config.name;
    vulkan->width = config.windowWidth;
    vulkan->height = config.windowHeight;
    vulkan->zNear = config.zNear;
    vulkan->zFar = config.zFar;
    vulkan->FoV = config.FoV;
    vulkan->mouseSensitivity = config.mouseSensitivity;
    vulkan->brdfPath = config.brdf;
    if (config.sync == true)
    {
        vulkan->defaultPresentMode = vk::PresentModeKHR::eFifo;
    }
    else
    {
        vulkan->defaultPresentMode = vk::PresentModeKHR::eMailbox;
    }

    // setup glfw window
    vulkan->window = std::make_shared<Window>(this, vulkan->width, vulkan->height, "Vulkans Eye");

    // setup input
    Input::getInstance();
    vulkan->window->setKeyCallBack(&Input::keyCallback);
    vulkan->window->setMouseButtonCallback(&Input::mouseButtonCallback);
    vulkan->window->setCursorPosCallback(&Input::cursorPosCallback);
    vulkan->window->setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    vulkan->window->setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // init engine
    engine.vulkan = vulkan;
    engine.init();

    camera = std::make_shared<Camera>(vulkan);
    player = std::make_shared<Player>(camera, config.playerConfigPath);
    materials = std::make_shared<Materials>(vulkan, config.materialsConfigPath);
    meshes = std::make_shared<Meshes>(vulkan, config.meshesConfigPath);
    backdrops = std::make_shared<Backdrops>(vulkan, camera, config.backdropsConfigPath);
    scene = std::make_shared<Scene>(vulkan, camera, player, materials, meshes, backdrops, config.sceneConfigPath);
    overlay = std::make_shared<Overlay>(vulkan, player, camera);

    // prepare engine
    engine.scene = scene;
    engine.overlay = overlay;
    engine.prepare();
}

void VulkansEye::run()
{
    debugLogger->info("Start Main Loop");
    float lastFrameTime = 0.0F;
    while (vulkan->window->shouldClose() == 0)
    {
        float now = Timer::time();
        float deltaTime = now - lastFrameTime;
        lastFrameTime = now;

        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)vulkan->width, (float)vulkan->height);
        io.DeltaTime = deltaTime;
        overlay->newFrame();
        overlay->updateBuffers();

        glfwPollEvents();
        handleInput(deltaTime);
        player->update(deltaTime);
        camera->setPosition(glm::vec3(-1.F, -1.F, -1.F) * player->position());
        camera->update();

        engine.drawFrame(deltaTime);
    }
    vkDeviceWaitIdle(vulkan->device);
    debugLogger->info("End Main Loop");
}

void VulkansEye::handleInput(float deltaTime)
{
    // game mode
    if (Input::wasKeyReleased(GLFW_KEY_F1))
    {
        if (vulkan->mode != Mode::Game)
        {
            if (displayMode == DisplayMode::cursor)
            {
                vulkan->window->setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                vulkan->window->setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
                camera->mouseMode = true;
                displayMode = DisplayMode::nocursor;
            }
            vulkan->showOverlay = false;
            vulkan->updateCommandBuffer = true;
            vulkan->mode = Mode::Game;
            debugLogger->info("Changed Mode to Game");
        }
    }
    // dbug mode
    if (Input::wasKeyReleased(GLFW_KEY_F2))
    {
        if (vulkan->mode != Mode::Dbug)
        {
            if (displayMode == DisplayMode::cursor)
            {
                vulkan->window->setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                vulkan->window->setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
                camera->mouseMode = true;
                displayMode = DisplayMode::nocursor;
            }
            vulkan->showOverlay = true;
            vulkan->updateCommandBuffer = true;
            vulkan->mode = Mode::Dbug;
        }
        debugLogger->info("Changed Mode to Dbug");
    }

    // nput mode
    if (Input::wasKeyReleased(GLFW_KEY_F3))
    {
        if (vulkan->mode != Mode::Nput)
        {
            if (displayMode != DisplayMode::cursor)
            {
                vulkan->window->setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                camera->mouseMode = false;
                displayMode = DisplayMode::cursor;
            }
            vulkan->mode = Mode::Nput;
            debugLogger->info("Changed Mode to Nput");
        }
    }

    camera->look(Input::getMouseX(), Input::getMouseY());

    auto moveDir = glm::vec2(0.F);
    moveDir.y -= static_cast<float>(Input::isKeyPressed(GLFW_KEY_W));
    moveDir.y += static_cast<float>(Input::isKeyPressed(GLFW_KEY_S));
    moveDir.x += static_cast<float>(Input::isKeyPressed(GLFW_KEY_A));
    moveDir.x -= static_cast<float>(Input::isKeyPressed(GLFW_KEY_D));
    player->move(moveDir, deltaTime);

    if (Input::isKeyPressed(GLFW_KEY_SPACE) != 0)
    {
        player->jump();
    }

    if (Input::wasKeyReleased(GLFW_KEY_ESCAPE))
    {
        debugLogger->info("Pressed Escape Closing");
        vulkan->window->setClose(1);
    }
}

} // namespace tat