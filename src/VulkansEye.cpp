#include "VulkansEye.hpp"
#include "Config.h"
#include "helpers.h"

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

    config.backdrop = j.value("backdrop", config.backdrop);

    if (j.find("lights") == j.end())
    {
        config.pointLights.resize(1); // loads defaults into index 0
    }
    else
    {
        //set size of lights equal to numberr of lights in config
        config.pointLights.resize(j.at("lights").size());
        //iterate over config and store value from json into config if that value exists
        //otherwise leave the value alone from the default constructed value
        int32_t i = 0;
        for (auto &[key, light] : j.at("lights").items())
        {
            config.pointLights[i].name = key;
            if (light.find("position") != light.end())
            {
                auto position = light.at("position");
                config.pointLights[i].position.x = position.value("x", config.pointLights[i].position.x);
                config.pointLights[i].position.y = position.value("y", config.pointLights[i].position.y);
                config.pointLights[i].position.z = position.value("z", config.pointLights[i].position.z);
            }
            config.pointLights[i].temperature = light.value("temperature", config.pointLights[i].temperature);
            config.pointLights[i].lumens = light.value("lumens", config.pointLights[i].lumens);
            ++i;
        }
    }

    if (j.find("materials") == j.end())
    {
        config.materials.resize(1);
    }
    else
    {
        config.materials.resize(j.at("materials").size());
        int32_t i = 0;
        for (auto &[key, material] : j.at("materials").items())
        {
            config.materials[i].name = key;
            config.materials[i].diffuse = material.value("diffuse", config.materials[i].diffuse);
            config.materials[i].normal = material.value("normal", config.materials[i].normal);
            config.materials[i].metallic = material.value("metallic", config.materials[i].metallic);
            config.materials[i].roughness = material.value("roughness", config.materials[i].roughness);
            config.materials[i].ao = material.value("ao", config.materials[i].ao);
            config.materials[i].displacement = material.value("displacement", config.materials[i].displacement);
            ++i;
        }
    }

    if (j.find("meshes") == j.end())
    {
        config.meshes.resize(1);
    }
    else
    {
        config.meshes.resize(j.at("meshes").size());
        int32_t i = 0;
        for (auto &[key, value] : j.at("meshes").items())
        {
            config.meshes[i].name = key;
            config.meshes[i].path = value;
            ++i;
        }
    }

    if (j.find("models") == j.end())
    {
        config.models.resize(1);
    }
    else
    {
        config.models.resize(j.at("models").size());
        int32_t i = 0;
        for (auto &[key, model] : j.at("models").items())
        {
            config.models[i].name = key;
            config.models[i].mesh = model.value("mesh", config.models[i].mesh);
            config.models[i].material = model.value("material", config.models[i].material);
            if (model.find("position") != model.end())
            {
                auto position = model.at("position");
                config.models[i].position.x = position.value("x", config.models[i].position.x);
                config.models[i].position.y = position.value("y", config.models[i].position.y);
                config.models[i].position.z = position.value("z", config.models[i].position.z);
            }
            if (model.find("rotation") != model.end())
            {
                auto rotation = model.at("rotation");
                config.models[i].rotation.x = rotation.value("x", config.models[i].rotation.x);
                config.models[i].rotation.y = rotation.value("y", config.models[i].rotation.y);
                config.models[i].rotation.z = rotation.value("z", config.models[i].rotation.z);
            }
            if (model.find("scale") != model.end())
            {
                auto scale = model.at("scale");
                config.models[i].scale.x = scale.value("x", config.models[i].scale.x);
                config.models[i].scale.y = scale.value("y", config.models[i].scale.y);
                config.models[i].scale.z = scale.value("z", config.models[i].scale.z);
            }
            if (model.find("tessellation") != model.end())
            {
                auto tessellation = model.at("tessellation");
                config.models[i].tessLevel = tessellation.value("level", config.models[i].tessLevel);
                config.models[i].tessStregth = tessellation.value("strength", config.models[i].tessStregth);
                config.models[i].tessAlpha = tessellation.value("alpha", config.models[i].tessAlpha);
            }
            ++i;
        }
    }
};

} // namespace tat