#include "VulkansEye.hpp"

VulkansEye::VulkansEye()
{
    windowWidth = 0;
    windowHeight = 0;
    backend = nullptr;
};

void VulkansEye::init(uint32_t width, uint32_t height)
{
    windowWidth = width;
    windowHeight = height;
    initWindow();

    Config config;
    config.skybox = {"resources/textures/skybox/nebula.dds"};

    CameraConfig camera;
    camera.fieldOfView = 60.0f;
    camera.position = {0.0f, 0.0f, -5.0f};
    camera.rotation = {0.0f, 0.0f, 0.0f};
    config.cameras = {camera};

    LightConfig light;
    light.id = 0;
    light.color = {1.0f, 1.0f, 1.0f};
    light.position = {5.0f, 5.0f, 5.0f};
    light.rotation = {0.0f, 0.0f, 0.0f};
    config.lights = {light};

    MeshConfig a;
    a.id = 1;
    a.objPath = "resources/models/a.obj";
    MeshConfig b;
    b.id = 2;
    b.objPath = "resources/models/b.obj";
    config.meshes = {a, b};

    MaterialConfig brick;
    brick.id = 1;
    brick.diffusePath = "resources/textures/brick/diffuse.png";
    brick.normalPath = "resources/textures/brick/normal.png";
    brick.roughnessPath = "resources/textures/brick/roughness.png";
    config.materials = {brick};

    ModelConfig letterA;
    letterA.id = 1;
    letterA.position = {1.0f, 0.25f, 0.5f};
    letterA.type = obj;
    letterA.meshId = 1;
    letterA.materialId = 1;
    ModelConfig letterB;
    letterB.id = 2;
    letterB.position = {0.0f, 0.25f, 0.5f};
    letterB.type = obj;
    letterB.meshId = 2;
    letterB.materialId = 1;
    config.models = {letterA, letterB};

    state = new State(window, width, height);
    scene = new Scene(state, config);
    backend = new VkBackend(state, scene);

    setupInputCallbacks();
    backend->initVulkan();
}

void VulkansEye::run()
{
    mainLoop();
    cleanup();
}

void VulkansEye::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(windowWidth, windowHeight, "Vulkans Eye", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void VulkansEye::setupInputCallbacks()
{
    Input &hexmap = Input::getInstance();

    glfwSetKeyCallback(window, &Input::keyCallback);
    glfwSetMouseButtonCallback(window, &Input::mouseButtonCallback);
    glfwSetCursorPosCallback(window, &Input::cursorPosCallback);
}

void VulkansEye::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        glfwPollEvents();

        if (Input::checkMouse(GLFW_MOUSE_BUTTON_2))
        {
            if (!scene->camera.mouseMode)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
            scene->camera.update(time);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            scene->camera.mouseMode = false;
        }

        backend->drawFrame();

        if (Input::checkKeyboard(GLFW_KEY_ESCAPE))
        {
            std::cerr << "Pressed Escape.  Closing." << std::endl;
            glfwSetWindowShouldClose(window, true);
        }
    }

    vkDeviceWaitIdle(state->device);
}

void VulkansEye::cleanup()
{
    delete backend;
    glfwDestroyWindow(window);

    glfwTerminate();
}
