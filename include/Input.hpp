#pragma once

#include <cstdint>
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <array>
#include <imgui.h>
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>   // for glfwGetWin32Window
#endif
#define GLFW_HAS_WINDOW_TOPMOST       (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3200) // 3.2+ GLFW_FLOATING
#define GLFW_HAS_WINDOW_HOVERED       (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300) // 3.3+ GLFW_HOVERED
#define GLFW_HAS_WINDOW_ALPHA         (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300) // 3.3+ glfwSetWindowOpacity
#define GLFW_HAS_PER_MONITOR_DPI      (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300) // 3.3+ glfwGetMonitorContentScale
#define GLFW_HAS_VULKAN               (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3200) // 3.2+ glfwCreateWindowSurface

// Singleton as there can only be one Input and Input should never be locked

namespace tat
{

// vimlike modes don't know what visual will do yet
enum class InputMode
{
    Normal,
    Insert,
    Visual
};

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

    static void switchMode(InputMode mode)
    {
        getInstance().switchModeImpl(mode);
    }

    void switchModeImpl(InputMode mode)
    {
        this->mode = mode;
    }

    static auto getMode() -> InputMode
    {
        return getInstance().getModeImpl();
    }

    auto getModeImpl() -> InputMode
    {
        return mode;
    }

    static void mouseButtonCallback(GLFWwindow *window, int button, int action,
                                    int mods) // this method is specified as glfw callback
    {
        // here we access the instance via the singleton pattern and forward the
        // callback to the instance method
        getInstance().mouseButtonCallbackImpl(window, button, action, mods);
    }

    // in array of mouse buttons the correspondin button is true while pressed
    void mouseButtonCallbackImpl(GLFWwindow * /*window*/, int button, int action, int /*mods*/)
    {
        if (mode == InputMode::Insert)
        {
            auto &io = ImGui::GetIO();
            if (action == GLFW_PRESS)
            {
                if (button >= 0 && button < 8)
                {
                    mouseButtons[button] = true;
                }
                io.MouseDown[button] = true;
            }
            else if (action == GLFW_RELEASE)
            {
                if (button >= 0 && button < 8)
                {
                    mouseButtons[button] = false;
                }
                io.MouseDown[button] = false;
            }
            return;
        }

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

        if (mode == InputMode::Insert)
        {
            auto &io = ImGui::GetIO();
            io.MousePos.x = static_cast<float>(xpos);
            io.MousePos.y = static_cast<float>(ypos);
        }
    }

    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        getInstance().keyCallbackImpl(window, key, scancode, action, mods);
    }

    void keyCallbackImpl(GLFWwindow * /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
    {
        if (mode == InputMode::Insert)
        {
            auto &io = ImGui::GetIO();
            if (action == GLFW_PRESS)
            {
                if (key >= 0 && key < 349)
                {
                    pressed[key] = 1;
                    released[key] = 0;
                }
                io.KeysDown[key] = true;
            }
            else if (action == GLFW_RELEASE)
            {
                if (key >= 0 && key < 349)
                {
                    pressed[key] = 0;
                    released[key] = 1;
                }
                io.KeysDown[key] = false;
            }
            io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
            io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
            io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
            io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
            return;
        }

        if (action == GLFW_PRESS)
        {
            if (key >= 0 && key < 349)
            {
                pressed[key] = 1;
                released[key] = 0;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            if (key >= 0 && key < 349)
            {
                pressed[key] = 0;
                released[key] = 1;
            }
        }
    }

    // return 0 if not pressed, 1 if pressed
    static auto isKeyPressed(const uint32_t key) -> int8_t
    {
        return getInstance().isKeyPressedIMPL(key);
    }

    auto isKeyPressedIMPL(const uint32_t key) -> int8_t
    {
        return pressed[key];
    }

    static auto wasKeyReleased(const uint32_t key) -> bool
    {
        return getInstance().wasKeyReleasedIMPL(key);
    }

    auto wasKeyReleasedIMPL(const uint32_t key) -> bool
    {
        if (released[key] != 0)
        {
            released[key] = 0;
            return true;
        }
        return false;
    }

    static void charCallback(GLFWwindow * /*window*/, unsigned int c)
    {
        ImGuiIO &io = ImGui::GetIO();
        io.AddInputCharacter(c);
    }

    static void scrollCallback(GLFWwindow * /*window*/, double xoffset, double yoffset)
    {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseWheelH += (float)xoffset;
        io.MouseWheel += (float)yoffset;
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

    // if button is being pressed return true
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
    std::array<bool, 9> mouseButtons{false};

    // GLFW_KEY_LAST == 348
    std::array<int8_t, 349> pressed{0};
    std::array<int8_t, 349> released{0};
    InputMode mode = InputMode::Normal;

    double mouseX{};
    double mouseY{};
};

} // namespace tat