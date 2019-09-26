#pragma once

#include <glm/glm.hpp>

#include "Helpers.h"

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

    void mouseButtonCallbackImpl(GLFWwindow *window, int key, int action, int mods) //this is the actual implementation of the callback method
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
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            std::cerr << "Pressed Escape\n";
            glfwSetWindowShouldClose(window, VK_TRUE);
        }
        else if (key == GLFW_KEY_A)
        {
            if (action == GLFW_PRESS)
            {
                keys.a = true;
            }
            else if (action == GLFW_RELEASE)
            {
                keys.a = false;
            }
        }
        else if (key == GLFW_KEY_D)
        {
            if (action == GLFW_PRESS)
            {
                keys.d = true;
            }
            else if (action == GLFW_RELEASE)
            {
                keys.d = false;
            }
        }
        else if (key == GLFW_KEY_W)
        {
            if (action == GLFW_PRESS)
            {
                keys.w = true;
            }
            else if (action == GLFW_RELEASE)
            {
                keys.w = false;
            }
        }
        else if (key == GLFW_KEY_S)
        {
            if (action == GLFW_PRESS)
            {
                keys.s = true;
            }
            else if (action == GLFW_RELEASE)
            {
                keys.s = false;
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

    static bool getMouseMode(){
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