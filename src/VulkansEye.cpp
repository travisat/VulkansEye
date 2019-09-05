#include "VulkansEye.h"

void VulkansEye::init(uint32_t width, uint32_t height, std::string tPath, std::string mPath)
{
    windowWidth = width; 
    windowHeight = height;
    initWindow();
    backend = new VkBackend(window, width, height, tPath, mPath);

    setupInputCallbacks();
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

