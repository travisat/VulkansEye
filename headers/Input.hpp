#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <array>
#include <iostream>


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Singleton as there can only be one Input and Input should never be locked

namespace tat
{

class Input
{
  public:
    Input(Input const &) = delete;          // prevent copies
    void operator=(Input const &) = delete; // prevent assignments

    static auto getInstance() -> Input & // Singleton is accessed via getInstance()
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

    void mouseButtonCallbackImpl(GLFWwindow * /*window*/, int button, int action, int /*mods*/)
    {
        if (action == GLFW_PRESS)
        {
            if (button >= 0 && button < 8)
            {
                mouseButtons[button] = true;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            if (button >= 0 && button < 8)
            {
                mouseButtons[button] = false;
            }
        }
    }

    static void cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
    {
        getInstance().cursorPosCallbackIMPL(window, xpos, ypos);
    }

    void cursorPosCallbackIMPL(GLFWwindow * /*window*/, double xpos, double ypos)
    {
        mouseX = xpos;
        mouseY = ypos;
    }

    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        getInstance().keyCallbackImpl(window, key, scancode, action, mods);
    }

    void keyCallbackImpl(GLFWwindow * /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
    {
        if (action == GLFW_PRESS)
        {
            if (key >= 0 && key < 349)
            {
                pressed[key] = true;
                released[key] = false;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            if (key >= 0 && key < 349)
            {
                pressed[key] = false;
                released[key] = true;
            }
        }
    }

    static auto isKeyPressed(const uint32_t key) -> bool
    {
        return getInstance().isKeyPressedIMPL(key);
    }

    auto isKeyPressedIMPL(const uint32_t key) -> bool
    {
        return pressed[key];
    }

    static auto wasKeyReleased(const uint32_t key) -> bool
    {
        return getInstance().wasKeyReleasedIMPL(key);
    }

    auto wasKeyReleasedIMPL(const uint32_t key) -> bool
    {
        if (released[key])
        {
            released[key] = false;
            return true;
        }
        return false;
    }

    static auto getMouseY() -> double
    {
        return getInstance().getMouseYIMPL();
    }

    auto getMouseYIMPL() -> double
    {
        return mouseY;
    }

    static auto getMouseX() -> double
    {
        return getInstance().getMouseXIMPL();
    }

    auto getMouseXIMPL() -> double
    {
        return mouseX;
    }

    static auto checkMouse(uint32_t button) -> bool
    {
        return getInstance().checkMouseIMPL(button);
    }

    auto checkMouseIMPL(uint32_t button) -> bool
    {
        return mouseButtons[button];
    }

  private:
    Input() = default; // private constructor necessary to allow only 1 instance

    // GLFW_MOUSE_BUTTON_LAST = 8
    std::array<bool, 9> mouseButtons = {false};

    // GLFW_KEY_LAST == 348
    std::array<bool, 349> pressed = {false};
    std::array<bool, 349> released = {false};

    double mouseX{};
    double mouseY{};
};

} // namespace tat