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
    config.materialPaths = {"resources/textures/wood.jpg", "resources/textures/stone.jpg"};
    config.modelPaths = {"resources/models/a.obj", "resources/models/b.obj"};
    config.objectIndices = {{0, 0}, {0, 1}};
    config.objectPositions = {{25, 25, 50}, {75, 50, 50}};
    config.skyboxTextures = {"resources/textures/skyboxxneg.jpg",
                             "resources/textures/skyboxxpos.jpg",
                             "resources/textures/skyboxyneg.jpg",
                             "resources/textures/skyboxypos.jpg",
                             "resources/textures/skyboxzneg.jpg",
                             "resources/textures/skyboxzpos.jpg"};

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
