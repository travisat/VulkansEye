#include "VulkansEye.h"

void VulkansEye::init(uint32_t width, uint32_t height)
{
    windowWidth = width; 
    windowHeight = height;
    initWindow();
    backend = new VkBackend(window, width, height);
    backend->initModels({"resources/models/a.obj","resources/models/b.obj"});
    backend->initMaterials({"resources/textures/wood.jpg","resources/textures/stone.jpg"});
    backend->initObjects({{0,0,25,50,50},{1,1,75,50,50}});
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

    vkDeviceWaitIdle(backend->device);
}

void VulkansEye::cleanup()
{
    delete backend;
    glfwDestroyWindow(window);

    glfwTerminate();
}

