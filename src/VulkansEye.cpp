#include "VulkansEye.hpp"

VulkansEye::VulkansEye()
{
    windowWidth = 0;
    windowHeight = 0;
    engine = nullptr;
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
    camera.position = {0.0f, 10.0f, 0.0f};
    camera.rotation = {0.0f, -34.0f, 0.0f};
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

    MeshConfig a;
    a.id = 1;
    a.objPath = "resources/models/a.obj";
    MeshConfig b;
    b.id = 2;
    b.objPath = "resources/models/b.obj";
    MeshConfig wallMesh;
    wallMesh.id = 3;
    wallMesh.objPath = "resources/models/wall.obj";
    config.meshes = {a, wallMesh};

    MaterialConfig brick;
    brick.id = 1;
    brick.diffusePath = "resources/textures/brick/diffuse.png";
    brick.normalPath = "resources/textures/brick/normal.png";
    brick.roughnessPath = "resources/textures/brick/roughness.png";
    brick.aoPath = "resources/textures/brick/ambientOcclusion.png";
    MaterialConfig alien;
    alien.id = 2;
    alien.diffusePath = "resources/textures/alien/diffuse.png";
    alien.normalPath = "resources/textures/alien/normal.png";
    alien.roughnessPath = "resources/textures/alien/roughness.png";
    alien.aoPath = "resources/textures/alien/ambientOcclusion.png";
    config.materials = {brick, alien};

    ModelConfig letterA;
    letterA.id = 1;
    letterA.position = {1.0f, 0.0f, 5.5f};
    letterA.scale = glm::vec3(1.0f);
    letterA.type = obj;
    letterA.meshId = 3;
    letterA.materialId = 1;

    ModelConfig wall;
    wall.id = 3;
    wall.position = {1.0f,0.0f,0.0f};
    wall.scale = glm::vec3(1.0f);
    wall.type = obj;
    wall.meshId = 1;
    wall.materialId = 2;
    config.models = {letterA, wall};

    state = new State(window, width, height);
    scene = new Scene(state, config);
    engine = new VkEngine(state, scene);

    setupInputCallbacks();
    engine->initVulkan();
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

        engine->drawFrame();

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
    delete engine;
    glfwDestroyWindow(window);

    glfwTerminate();
}
