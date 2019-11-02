#include "VulkansEye.hpp"
#include "Config.h"
#include "helpers.h"
#include "vulkan/vulkan.hpp"

namespace tat
{

void VulkansEye::init(const std::string &configPath)
{
    // start timers
    Timer::getInstance();
    Timer::time();
    Timer::systemTime();

    // load config
    Config config;
    loadConfig(configPath, config);

    // load display settings
    vulkan.name = config.vulkan.name;
    vulkan.width = config.vulkan.windowWidth;
    vulkan.height = config.vulkan.windowHeight;
    vulkan.zNear = config.vulkan.zNear;
    vulkan.zFar = config.vulkan.zFar;
    if (config.vulkan.sync == true)
    {
        vulkan.defaultPresentMode = vk::PresentModeKHR::eFifo;
    }
    else
    {
        vulkan.defaultPresentMode = vk::PresentModeKHR::eMailbox;
    }

    // setup glfw window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    vulkan.window = glfwCreateWindow(vulkan.width, vulkan.height, "Vulkans Eye", nullptr, nullptr);
    glfwSetWindowUserPointer(vulkan.window, this);
    glfwSetFramebufferSizeCallback(vulkan.window, framebufferResizeCallback);

    // setup input
    Input::getInstance();
    glfwSetKeyCallback(vulkan.window, &Input::keyCallback);
    glfwSetMouseButtonCallback(vulkan.window, &Input::mouseButtonCallback);
    glfwSetCursorPosCallback(vulkan.window, &Input::cursorPosCallback);
    glfwSetInputMode(vulkan.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(vulkan.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    displayMode = DisplayMode::nocursor;

    // create player
    player.config = &config.player;
    player.vulkan = &vulkan;
    player.create();

    // load scene
    scene.config = &config;
    scene.vulkan = &vulkan;
    scene.player = &player;

    // load overlay
    overlay.vulkan = &vulkan;
    overlay.player = &player;

    // start engine
    engine.vulkan = &vulkan;
    engine.scene = &scene;
    engine.overlay = &overlay;
    engine.init();
}

void VulkansEye::run()
{
    mainLoop();
    cleanup();
}

void VulkansEye::cleanup()
{
    glfwDestroyWindow(vulkan.window);
    glfwTerminate();
}

void VulkansEye::mainLoop()
{
    float lastFrameTime = 0.0F;
    while (glfwWindowShouldClose(vulkan.window) == 0)
    {
        float now = Timer::time();
        float deltaTime = now - lastFrameTime;
        lastFrameTime = now;

        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)vulkan.width, (float)vulkan.height);
        io.DeltaTime = deltaTime;
        overlay.newFrame();
        overlay.updateBuffers();

        glfwPollEvents();
        handleInput();

        player.update(deltaTime);

        engine.drawFrame();
    }
    vkDeviceWaitIdle(vulkan.device);
}

void VulkansEye::handleInput()
{
    if (Input::wasKeyReleased(GLFW_KEY_F1))
    {
        if (displayMode == DisplayMode::cursor)
        {
            glfwSetInputMode(vulkan.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetInputMode(vulkan.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            player.mouseMode = true;
            displayMode = DisplayMode::nocursor;
        }
        else if (displayMode == DisplayMode::nocursor)
        {
            glfwSetInputMode(vulkan.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            player.mouseMode = false;
            displayMode = DisplayMode::cursor;
        }
    }
    if (Input::wasKeyReleased(GLFW_KEY_ESCAPE))
    {
        std::cerr << "Pressed Escape.  Closing." << std::endl;
        glfwSetWindowShouldClose(vulkan.window, 1);
    }
    if (Input::wasKeyReleased(GLFW_KEY_F3))
    {
        vulkan.showOverlay = !vulkan.showOverlay;
        vulkan.updateCommandBuffer = true;
    }

    auto moveDir = glm::vec2(0.0F);
    if (Input::isKeyPressed(GLFW_KEY_W))
    {
        moveDir.y += 1.0F;
    }
    if (Input::isKeyPressed(GLFW_KEY_S))
    {
        moveDir.y -= 1.0F;
    }
    if (Input::isKeyPressed(GLFW_KEY_A))
    {
        moveDir.x -= 1.0F;
    }
    if (Input::isKeyPressed(GLFW_KEY_D))
    {
        moveDir.x += 1.0F;
    }
    player.move(moveDir);

    if (Input::isKeyPressed(GLFW_KEY_SPACE))
    {
        player.jump();
    }
}

void VulkansEye::loadConfig(const std::string &path, Config &config)
{
    std::ifstream file(path);
    json j;
    file >> j;

    if (j.find("vulkan") != j.end())
    {
        auto vulkan = j.at("vulkan");
        config.vulkan.name = vulkan.value("name", config.vulkan.name);
        config.vulkan.zNear = vulkan.value("zNear", config.vulkan.zNear);
        config.vulkan.zFar = vulkan.value("zFar", config.vulkan.zFar);
        config.vulkan.windowWidth = vulkan.value("windowWidth", config.vulkan.windowWidth);
        config.vulkan.windowHeight = vulkan.value("windowHeight", config.vulkan.windowHeight);
        config.vulkan.sync = vulkan.value("sync", config.vulkan.sync);
        config.vulkan.shadowSize = vulkan.value("shadowSize", config.vulkan.shadowSize);
    }

    if (j.find("player") != j.end())
    {
        auto player = j.at("player");
        config.player.height = player.value("height", config.player.height);
        config.player.mass = player.value("mass", config.player.mass);
        config.player.velocityMax = player.value("velocityMax", config.player.velocityMax);
        config.player.timeToReachVMax = player.value("timeToReachVMax", config.player.timeToReachVMax);
        config.player.timeToStopfromVMax = player.value("timeToStopFromVMax", config.player.timeToStopfromVMax);
        config.player.jumpHeight = player.value("jumpHeight", config.player.jumpHeight);
        config.player.mouseSensitivity = player.value("mouseSensitivity", config.player.mouseSensitivity);
        config.player.fieldOfView = player.value("fieldOfView", config.player.fieldOfView);
        if (player.find("position") != player.end())
        {
            auto position = player.at("position");
            config.player.rotation.x = position.value("x", config.player.position.x);
            config.player.position.y = position.value("y", config.player.position.y);
            config.player.position.z = position.value("z", config.player.position.z);
        }
        if (player.find("rotation") != player.end())
        {
            auto rotation = player.at("rotation");
            config.player.rotation.x = rotation.value("x", config.player.rotation.x);
            config.player.rotation.y = rotation.value("y", config.player.rotation.y);
            config.player.rotation.z = rotation.value("z", config.player.rotation.z);
        }
    }

    if (j.find("backdrop") != j.end())
    {
        auto backdrop = j.at("backdrop");
        config.backdrop.colorPath = backdrop.value("color", config.backdrop.colorPath);
        config.backdrop.radiancePath = backdrop.value("radiance", config.backdrop.radiancePath);
        config.backdrop.irradiancePath = backdrop.value("irradiance", config.backdrop.irradiancePath);
        if (backdrop.find("light") != backdrop.end())
        {
            auto light = backdrop.at("light");
            if (light.find("position") != light.end())
            {
                auto position = light.at("position");
                config.backdrop.light.position.x = position.value("x", config.backdrop.light.position.x);
                config.backdrop.light.position.y = position.value("y", config.backdrop.light.position.y);
                config.backdrop.light.position.z = position.value("z", config.backdrop.light.position.z);
            }
            config.backdrop.light.steradians =
                light.value("steradians", config.backdrop.light.steradians);
            config.backdrop.light.temperature =
                light.value("temperature", config.backdrop.light.temperature);
            config.backdrop.light.lumens = light.value("lumens", config.backdrop.light.lumens);
        }
    }

    if (j.find("lights") == j.end())
    {
        config.lights.resize(1); // loads defaults into index 0
    }
    else
    {
        for (auto &[key, light] : j.at("lights").items())
        {
            LightConfig c{};
            c.name = key;
            if (light.find("position") != light.end())
            {
                auto position = light.at("position");
                c.position.x = position.value("x", c.position.x);
                c.position.y = position.value("y", c.position.y);
                c.position.z = position.value("z", c.position.z);
            }
            c.steradians = light.value("steradians", c.steradians);
            c.temperature = light.value("temperature", c.temperature);
            c.lumens = light.value("lumens", c.lumens);
            config.lights.push_back(c);
        }
    }

    if (j.find("materials") == j.end())
    {
        config.materials.resize(1);
    }
    else
    {
        for (auto &[key, material] : j.at("materials").items())
        {
            MaterialConfig c{};
            c.name = key;
            c.diffuse = material.value("diffuse", c.diffuse);
            c.normal = material.value("normal", c.normal);
            c.metallic = material.value("metallic", c.metallic);
            c.roughness = material.value("roughness", c.roughness);
            c.ao = material.value("ao", c.ao);
            c.displacement = material.value("displacement", c.displacement);
            config.materials.push_back(c);
        }
    }

    if (j.find("meshes") == j.end())
    {
        config.meshes.resize(1);
    }
    else
    {
        for (auto &[key, value] : j.at("meshes").items())
        {
            MeshConfig c{};
            c.name = key;
            c.path = value;
            config.meshes.push_back(c);
        }
    }

    if (j.find("models") == j.end())
    {
        config.models.resize(1);
    }
    else
    {
        for (auto &[key, model] : j.at("models").items())
        {
            ModelConfig c{};
            c.name = key;
            c.mesh = model.value("mesh", c.mesh);
            c.material = model.value("material", c.material);
            if (model.find("rotation") != model.end())
            {
                auto rotation = model.at("rotation");
                c.rotation.x = rotation.value("x", c.rotation.x);
                c.rotation.y = rotation.value("y", c.rotation.y);
                c.rotation.z = rotation.value("z", c.rotation.z);
            }
            if (model.find("scale") != model.end())
            {
                auto scale = model.at("scale");
                c.scale.x = scale.value("x", c.scale.x);
                c.scale.y = scale.value("y", c.scale.y);
                c.scale.z = scale.value("z", c.scale.z);
            }
            if (model.find("type") == model.end() || model.find("type").value() == "single")
            {
                if (model.find("position") != model.end())
                {
                    auto position = model.at("position");
                    c.position.x = position.value("x", c.position.x);
                    c.position.y = position.value("y", c.position.y);
                    c.position.z = position.value("z", c.position.z);
                }

                config.models.push_back(c);
            }
            else if (model.find("type").value() == "multiple")
            {
                if (model.find("positions") != model.end())
                {
                    for (auto &position : model.at("positions"))
                    {
                        c.position.x = position.value("x", c.position.x);
                        c.position.y = position.value("y", c.position.y);
                        c.position.z = position.value("z", c.position.z);
                        config.models.push_back(c);
                    }
                }
            }
        }
    }
};

} // namespace tat