#include "VulkansEye.hpp"
#include "macros.h"

void VulkansEye::init(uint32_t width, uint32_t height)
{
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

    //setup config
    //TODO load from config file
    Config config{};
    config.skybox = {"resources/textures/skybox/nebula.dds"};

    CameraConfig camera;
    camera.fieldOfView = 60.0f;
    camera.position = {9.0f, 12.0f, -3.0f};
    camera.rotation = {-46.0f, 87.0f, 0.0f};
    config.cameras = {camera};

    const float p = 15.0f;
    LightConfig light0;
    light0.id = 0;
    light0.light = {-p, -p * 0.5f, -p, 1.0f};
    LightConfig light1;
    light1.id = 1;
    light0.light = {-p, -p * 0.5f, p, 1.0f};
    LightConfig light2;
    light2.id = 2;
    light0.light = {p, -p * 0.5f, p, 1.0f};
    LightConfig light3;
    light3.id = 3;
    light0.light = {p, -p * 0.5f, -p, 1.0f};

    config.lights = {light0, light1};

    ModelConfig wall;
    wall.id = 1;
    wall.position = {1.0f, 0.0f, 5.5f};
    wall.scale = glm::vec3(1.0f);
    wall.type = ModelType::obj;
    wall.materialConfig.id = 1;
    wall.materialConfig.type = ImageType::png;
    wall.materialConfig.diffusePath = "resources/textures/alien/diffuse.png";
    wall.materialConfig.normalPath = "resources/textures/alien/normal.png";
    wall.materialConfig.roughnessPath = "resources/textures/alien/roughness.png";
    wall.materialConfig.ambientOcclusionPath = "resources/textures/alien/ambientOcclusion.png";
    wall.meshConfig.id = 1;
    wall.meshConfig.objPath = "resources/models/wall.obj";

    config.models = {wall};

    //setup scene
    scene.config = &config;
    scene.vulkan = &vulkan;

    //setup overlay
    imgui.vulkan = &vulkan;
    imgui.cameraPosition = &scene.camera.position;
    imgui.cameraRotation = &scene.camera.rotation;
    imgui.init();

    //setup engine
    engine.vulkan = &vulkan;
    engine.scene = &scene;
    engine.imgui = &imgui;
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
        glfwPollEvents();

        if (Input::checkMouse(GLFW_MOUSE_BUTTON_2))
        {
            if (!scene.camera.mouseMode)
            {
                glfwSetInputMode(vulkan.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetInputMode(vulkan.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
            scene.camera.update(Timer::getInstance().getCount() / 1000.0f);
        }
        else
        {
            glfwSetInputMode(vulkan.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            scene.camera.mouseMode = false;
        }

        ImGuiIO &io = ImGui::GetIO();

        io.DisplaySize = ImVec2((float)vulkan.width, (float)vulkan.height);
        io.DeltaTime = std::chrono::duration<float, std::chrono::seconds::period>(Timer::getTime() - lastFrameTime).count();
        lastFrameTime = Timer::getTime();

        imgui.newFrame(vulkan.frameCounter == 0);
        imgui.updateBuffers();

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
