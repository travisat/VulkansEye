#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct Keys
{
    bool a = false;
    bool d = false;
    bool w = false;
    bool s = false;
};

// Input.h (the actual callback class for glfwSetMouseButtonCallback)
class Input
{
public:
    static Input &getInstance() // Singleton is accessed via getInstance()
    {
        static Input instance; // lazy singleton, instantiated on first use
        return instance;
    }

    static void mouseButtonCallback(GLFWwindow *window, int key, int action, int mods) // this method is specified as glfw callback
    {
        //here we access the instance via the singleton pattern and forward the callback to the instance method
        getInstance().mouseButtonCallbackImpl(window, key, action, mods);
    }

    void mouseButtonCallbackImpl(GLFWwindow *window, int key, int action, int mods)
    {
        if (key == GLFW_MOUSE_BUTTON_2)
        {
            if (action == GLFW_PRESS)
            {
                mouseButton2Pressed = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwGetCursorPos(window, &mouseX, &mouseY);
            }
            else if (action == GLFW_RELEASE)
            {
                mouseButton2Pressed = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }

    static void cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
    {
        getInstance().cursorPosCallbackIMPL(window, xpos, ypos);
    }

    void cursorPosCallbackIMPL(GLFWwindow *window, double xpos, double ypos)
    {
        mouseX = xpos;
        mouseY = ypos;
    }

    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        getInstance().keyCallbackImpl(window, key, scancode, action, mods);
    }

    void keyCallbackImpl(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)
        {
            switch (key)
            {
            case GLFW_KEY_ESCAPE:
                std::cerr << "Pressed Escape\n";
                glfwSetWindowShouldClose(window, VK_TRUE);
                break;
            case GLFW_KEY_A:
                keys.a = true;
                break;
            case GLFW_KEY_S:
                keys.s = true;
                break;
            case GLFW_KEY_D:
                keys.d = true;
                break;
            case GLFW_KEY_W:
                keys.w = true;
                break;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            switch (key)
            {
            case GLFW_KEY_A:
                keys.a = false;
                break;
            case GLFW_KEY_S:
                keys.s = false;
                break;
            case GLFW_KEY_D:
                keys.d = false;
                break;
            case GLFW_KEY_W:
                keys.w = false;
                break;
            }
        }
    }

    static Keys getKeys()
    {
        return getInstance().getKeysIMPL();
    }

    Keys getKeysIMPL()
    {
        return keys;
    }

    static double getMouseY()
    {
        return getInstance().getMouseYIMPL();
    }

    double getMouseYIMPL()
    {
        return mouseY;
    }

    static double getMouseX()
    {
        return getInstance().getMouseXIMPL();
    }

    double getMouseXIMPL()
    {
        return mouseX;
    }

    static bool getMouseMode()
    {
        return getInstance().getMouseModeIMPL();
    }

    bool getMouseModeIMPL()
    {
        return mouseButton2Pressed;
    }

private:
    Input(void) // private constructor necessary to allow only 1 instance
    {
    }

    Keys keys;

    bool mouseButton2Pressed = false;
    double mouseX;
    double mouseY;

    Input(Input const &);          // prevent copies
    void operator=(Input const &); // prevent assignments
};