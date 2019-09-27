#include "VulkansEye.h"

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

    MeshConfig a;
    a.id = 1;
    a.objPath= "resources/models/a.obj";

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
    letterA.meshId = 1;
    letterA.materialId = 1;
    letterA.xpos = 25;
    letterA.ypos = 25;
    letterA.zpos = 50;

    ModelConfig letterB;
    letterB.id = 2;
    letterB.meshId = 2;
    letterB.materialId = 1;
    letterB.xpos = 75;
    letterB.ypos = 50;
    letterB.zpos = 50;

    config.modelConfigs = {letterA, letterB};


    backend = new VkBackend(window, width, height, config);
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
        glfwPollEvents();
        backend->drawFrame();
    }

    vkDeviceWaitIdle(backend->state->device);
}

void VulkansEye::cleanup()
{
    delete backend;
    glfwDestroyWindow(window);

    glfwTerminate();
}
