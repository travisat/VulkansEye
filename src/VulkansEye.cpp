#include "VulkansEye.hpp"
#include "helpers.h"

namespace tat
{

void VulkansEye::init(uint32_t width, uint32_t height)
{
    vulkan.name = "Vulkans Eye";
    vulkan.width = width;
    vulkan.height = height;

    //setup timers
    Timer::getInstance();
    Timer::systemTime();

    //setup glfw window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    vulkan.window = glfwCreateWindow(vulkan.width, vulkan.height, "Vulkans Eye", nullptr, nullptr);
    glfwSetWindowUserPointer(vulkan.window, this);
    glfwSetFramebufferSizeCallback(vulkan.window, framebufferResizeCallback);

    //setup input
    Input::getInstance();
    glfwSetKeyCallback(vulkan.window, &Input::keyCallback);
    glfwSetMouseButtonCallback(vulkan.window, &Input::mouseButtonCallback);
    glfwSetCursorPosCallback(vulkan.window, &Input::cursorPosCallback);

    //load scene config
    SceneConfig config;
    loadSceneConfig("resources/configs/default.json", config);
    scene.config = &config;
    scene.vulkan = &vulkan;

    player.setPerspective(config.playerConfig.fieldOfView, static_cast<double>(vulkan.width), static_cast<double>(vulkan.height), 0.1f, 512.0f);
    player.position = config.playerConfig.position;
    player.rotation = config.playerConfig.rotation;
    player.height = config.playerConfig.height;
    player.mass = config.playerConfig.mass;
    player.jForce = config.playerConfig.jForce;
    player.velocityMax = config.playerConfig.velocityMax;
    player.timeToReachVMax = config.playerConfig.timeToReachVMax;
    player.timeToStopfromVMax = config.playerConfig.timeToStopfromVMax;
    player.mouseSensitivity = config.playerConfig.mouseSensitivity;
    player.updateView();
    scene.player = &player;

    //setup overlay
    overlay.vulkan = &vulkan;
    overlay.player = &player;

    //setup engine
    engine.vulkan = &vulkan;
    engine.scene = &scene;
    engine.overlay = &overlay;
    engine.init();
}

void VulkansEye::run()
{
    mainLoop();
    cleanup();
}

void VulkansEye::mainLoop()
{
    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(vulkan.window))
    {
        auto deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(Timer::getTime() - lastFrameTime).count();
        lastFrameTime = Timer::getTime();

        glfwPollEvents();

        if (Input::checkMouse(GLFW_MOUSE_BUTTON_2))
        {
            if (!player.mouseMode)
            {
                glfwSetInputMode(vulkan.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetInputMode(vulkan.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
                player.mouseMode = true;
            }
        }
        else
        {
            glfwSetInputMode(vulkan.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            player.mouseMode = false;
        }

        player.update(deltaTime);

        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)vulkan.width, (float)vulkan.height);
        io.DeltaTime = deltaTime;

        overlay.newFrame();

        overlay.updateBuffers();

        engine.drawFrame();
        vulkan.frameCounter++;

        if (Input::checkKeyboard(GLFW_KEY_ESCAPE))
        {
            std::cerr << "Pressed Escape.  Closing." << std::endl;
            glfwSetWindowShouldClose(vulkan.window, true);
        }
    }
    vkDeviceWaitIdle(vulkan.device);
}

void VulkansEye::cleanup()
{
    glfwDestroyWindow(vulkan.window);
    glfwTerminate();
}

} //namespace tat