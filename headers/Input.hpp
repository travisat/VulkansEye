#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Input.h (the actual callback class for glfwSetMouseButtonCallback)
// Singleton as there can only be one Input and Input should never be locked

namespace tat
{

class Input
{
  public:
    static Input &getInstance() // Singleton is accessed via getInstance()
    {
        static Input instance; // lazy singleton, instantiated on first use
        return instance;
    }

    static void mouseButtonCallback(GLFWwindow *window, int button, int action,
                                    int mods) // this method is specified as glfw callback
    {
        // here we access the instance via the singleton pattern and forward the
        // callback to the instance method
        getInstance().mouseButtonCallbackImpl(window, button, action, mods);
    }

    void mouseButtonCallbackImpl(GLFWwindow *window, int button, int action, int mods)
    {
        if (action == GLFW_PRESS)
        {
            if (button >= 0 && button < 8)
                mouseButtons[button] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            if (button >= 0 && button < 8)
                mouseButtons[button] = false;
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
            if (key >= 0 && key < 349)
                keys[key] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            if (key >= 0 && key < 349)
                keys[key] = false;
        }
    }

    static bool checkKeyboard(const uint32_t key)
    {
        return getInstance().checkKeyboardIMPL(key);
    }

    bool checkKeyboardIMPL(const uint32_t key)
    {
        return keys[key];
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

    static bool checkMouse(uint32_t button)
    {
        return getInstance().checkMouseIMPL(button);
    }

    bool checkMouseIMPL(uint32_t button)
    {
        return mouseButtons[button];
    }

  private:
    Input(void) // private constructor necessary to allow only 1 instance
    {
    }

    // GLFW_MOUSE_BUTTON_LAST = 8
    bool mouseButtons[8] = {false};

    // GLFW_KEY_LAST == 348
    bool keys[349] = {false};

    double mouseX;
    double mouseY;

    Input(Input const &);          // prevent copies
    void operator=(Input const &); // prevent assignments
};

} // namespace tat