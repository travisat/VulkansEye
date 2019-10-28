#include "VulkansEye.hpp"
#include "Config.h"
#include "helpers.h"

namespace tat
{

void VulkansEye::init()
{
    // start timers
    Timer::getInstance();
    Timer::time();
    Timer::systemTime();

    // load config
    Config config;
    loadConfig("resources/configs/default.json", config);

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
        VulkanConfig defaultConfig{};
        auto vulkan = j.find("vulkan").value();
        config.vulkan.name = vulkan.value("name",defaultConfig.name);
        config.vulkan.zNear = vulkan.value("zNear", defaultConfig.zNear);
        config.vulkan.zFar = vulkan.value("zFar", defaultConfig.zFar);
        config.vulkan.windowWidth = vulkan.value("windowWidth", defaultConfig.windowWidth);
        config.vulkan.windowHeight = vulkan.value("windowHeight", defaultConfig.windowHeight);
    }

    if (j.find("player") != j.end())
    {
        PlayerConfig defaultConfig{};
        auto player = j.find("player").value();
        config.player.height = player.value("height", defaultConfig.height);
        config.player.mass = player.value("mass", defaultConfig.mass);
        config.player.velocityMax = player.value("velocityMax", defaultConfig.velocityMax);
        config.player.timeToReachVMax = player.value("timeToReachVMax", defaultConfig.timeToReachVMax);
        config.player.timeToStopfromVMax = player.value("timeToStopFromVMax", defaultConfig.timeToStopfromVMax);
        config.player.jumpHeight = player.value("jumpHeight", defaultConfig.jumpHeight);
        config.player.mouseSensitivity = player.value("mouseSensitivity", defaultConfig.mouseSensitivity);
        config.player.fieldOfView = player.value("fieldOfView", defaultConfig.fieldOfView);
        if (player.find("position") != player.end())
        {
            auto position = player.find("position").value();
            config.player.rotation.x = position.value("x", defaultConfig.position.x);
            config.player.position.y = position.value("y", defaultConfig.position.y);
            config.player.position.z = position.value("z", defaultConfig.position.z);
        }
        if (player.find("rotation") != player.end())
        {
            auto rotation = player.find("rotation").value();
            config.player.rotation.x = rotation.value("x", defaultConfig.rotation.x);
            config.player.rotation.y = rotation.value("y", defaultConfig.rotation.y);
            config.player.rotation.z = rotation.value("z", defaultConfig.rotation.z);
        }
    }

    if (j.find("lights") == j.end())
    {
        config.pointLights.resize(1); // loads defaults into index 0
    }
    else
    {
        PointLightConfig defaultConfig{};
        config.pointLights.resize(j.at("lights").size());
        int32_t i = 0;
        for (auto &[key, lightconfig] : j.at("lights").items())
        {
            config.pointLights[i].index = i;
            config.pointLights[i].name = key;

            if (lightconfig.find("position") != lightconfig.end())
            {
                auto position = lightconfig.find("position").value();
                config.pointLights[i].position.x = position.value("x", defaultConfig.position.x);
                config.pointLights[i].position.y = position.value("y", defaultConfig.position.y);
                config.pointLights[i].position.z = position.value("z", defaultConfig.position.z);
            }
            config.pointLights[i].temperature = lightconfig.value("temperature", defaultConfig.temperature);
            config.pointLights[i].lumens = lightconfig.value("lumens", defaultConfig.lumens);
            ++i;
        }
    }

    if (j.find("materials") == j.end())
    {
        config.materials.material.resize(1);
    }
    else
    {
        MaterialConfig defaultConfig{};
        config.materials.material.resize(j.at("materials").size());
        int32_t i = 0;
        for (auto &[key, materialConfig] : j.at("materials").items())
        {
            config.materials.material[i].name = key;
            config.materials.material[i].diffuse = materialConfig.value("diffuse", defaultConfig.diffuse);
            config.materials.material[i].normal = materialConfig.value("normal", defaultConfig.normal);
            config.materials.material[i].metallic = materialConfig.value("metallic", defaultConfig.metallic);
            config.materials.material[i].roughness = materialConfig.value("roughness", defaultConfig.roughness);
            config.materials.material[i].ao = materialConfig.value("ao", defaultConfig.ao);
            config.materials.material[i].displacement =
                materialConfig.value("displacement", defaultConfig.displacement);
            ++i;
        }
    }

    if (j.find("actors") != j.end())
    {
        ModelConfig defaultConfig{};
        config.actors.resize(j["actors"].size());
        int32_t i = 0;
        for (auto &[key, actorconfig] : j.at("actors").items())
        {
            config.actors[i].index = i;
            config.actors[i].name = key;
            config.actors[i].model.object = actorconfig.value("object", defaultConfig.object);
            config.actors[i].model.material = actorconfig.value("material", defaultConfig.material);
            if (actorconfig.find("position") != actorconfig.end())
            {
                auto position = actorconfig.find("position").value();
                config.actors[i].model.position.x = position.value("x", defaultConfig.position.x);
                config.actors[i].model.position.y = position.value("y", defaultConfig.position.y);
                config.actors[i].model.position.z = position.value("z", defaultConfig.position.z);
            }
            if (actorconfig.find("rotation") != actorconfig.end())
            {
                auto rotation = actorconfig.find("rotation").value();
                config.actors[i].model.rotation.x = rotation.value("x", defaultConfig.rotation.x);
                config.actors[i].model.rotation.y = rotation.value("y", defaultConfig.rotation.y);
                config.actors[i].model.rotation.z = rotation.value("z", defaultConfig.rotation.z);
            }
            if (actorconfig.find("scale") != actorconfig.end())
            {
                auto scale = actorconfig.find("scale").value();
                config.actors[i].model.scale.x = scale.value("x", defaultConfig.scale.x);
                config.actors[i].model.scale.y = scale.value("y", defaultConfig.scale.y);
                config.actors[i].model.scale.z = scale.value("z", defaultConfig.scale.z);
            }
            if (actorconfig.find("tessellation") != actorconfig.end())
            {
                auto tessellation = actorconfig.find("tessellation").value();
                config.actors[i].model.tessLevel = tessellation.value("level", defaultConfig.tessLevel);
                config.actors[i].model.tessStregth = tessellation.value("strength", defaultConfig.tessStregth);
                config.actors[i].model.tessAlpha = tessellation.value("alpha", defaultConfig.tessAlpha);
            }
            ++i;
        }
    }

    if (j.find("stage") != j.end())
    {
        auto stage = j.find("stage").value();

        if (stage.find("backdrop") != stage.end())
        {
            config.stage.backdrop = stage.find("backdrop").value();
        }

        ModelConfig defaultConfig{};
        config.stage.models.resize(stage["models"].size());
        int32_t i = 0;
        for (auto &stagemodel : stage.at("models"))
        {
            config.stage.models[i].index = i;
            config.stage.models[i].object = stagemodel.value("object", defaultConfig.object);
            config.stage.models[i].material = stagemodel.value("material", defaultConfig.material);
            if (stagemodel.find("position") != stagemodel.end())
            {
                auto position = stagemodel.find("position").value();
                config.stage.models[i].position.x = position.value("x", defaultConfig.position.x);
                config.stage.models[i].position.y = position.value("y", defaultConfig.position.y);
                config.stage.models[i].position.z = position.value("z", defaultConfig.position.z);
            }
            if (stagemodel.find("rotation") != stagemodel.end())
            {
                auto rotation = stagemodel.find("rotation").value();
                config.stage.models[i].rotation.x = rotation.value("x", defaultConfig.rotation.x);
                config.stage.models[i].rotation.y = rotation.value("y", defaultConfig.rotation.y);
                config.stage.models[i].rotation.z = rotation.value("z", defaultConfig.rotation.z);
            }
            if (stagemodel.find("scale") != stagemodel.end())
            {
                auto scale = stagemodel.find("scale").value();
                config.stage.models[i].scale.x = scale.value("x", defaultConfig.scale.x);
                config.stage.models[i].scale.y = scale.value("y", defaultConfig.scale.y);
                config.stage.models[i].scale.z = scale.value("z", defaultConfig.scale.z);
            }
            if (stagemodel.find("tessellation") != stagemodel.end())
            {
                auto tessellation = stagemodel.find("tessellation").value();
                config.stage.models[i].tessLevel = tessellation.value("level", defaultConfig.tessLevel);
                config.stage.models[i].tessStregth = tessellation.value("strength", defaultConfig.tessStregth);
                config.stage.models[i].tessAlpha = tessellation.value("alpha", defaultConfig.tessAlpha);
            }
            ++i;
        }
    }
};

} // namespace tat