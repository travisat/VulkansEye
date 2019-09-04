#include <glm/glm.hpp>

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
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        std::cerr << "Mouse Button pressed at x: " << xpos << " y: " <<  ypos << std::endl;

    }

    static void cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
    {
        getInstance().cursorPosCallbackImpl(window, xpos, ypos);
    }

    void cursorPosCallbackImpl(GLFWwindow *window, double xpos, double ypos)
    {
        std::cerr << "Mouse at " << xpos << ", " << ypos << std::endl;
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        getInstance().keyCallbackImpl(window, key, scancode, action, mods);
    }

    void keyCallbackImpl(GLFWwindow * window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            std::cerr << "Pressed Escape\n";
            glfwSetWindowShouldClose(window, VK_TRUE);
        }
    }

private:
    Input(void) // private constructor necessary to allow only 1 instance
    {
    }

    Input(Input const &);          // prevent copies
    void operator=(Input const &); // prevent assignments
};