#include "VulkansEye.hpp"
#include "helpers.h"

namespace tat
{

void VulkansEye::init(uint32_t width, uint32_t height)
{
    vulkan.name = "Vulkans Eye";
    vulkan.width = width;
    vulkan.height = height;

    // setup timers
    Timer::getInstance();
    Timer::systemTime();

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

    // load scene config
    SceneConfig config;
    loadSceneConfig("resources/configs/default.json", config);

    vulkan.zNear = config.vulkanConfig.zNear;
    vulkan.zFar = config.vulkanConfig.zFar;

    scene.config = &config;
    scene.vulkan = &vulkan;
    scene.player = &player;

    // load player
    player.config = &config.playerConfig;
    player.vulkan = &vulkan;
    player.create();

    // setup overlay
    overlay.vulkan = &vulkan;
    overlay.player = &player;

    // setup engine
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

void VulkansEye::loadSceneConfig(const std::string& path, SceneConfig &config)
{
    std::ifstream file(path);
    json j;
    file >> j;

    j.at("index").get_to(config.index);
    j.at("name").get_to(config.name);

    j.at("vulkan").at("zNear").get_to(config.vulkanConfig.zNear);
    j.at("vulkan").at("zFar").get_to(config.vulkanConfig.zFar);

    config.actors.resize(j["actors"].size());
    int32_t i = 0;
    for (auto &actorconfig : j.at("actors"))
    {
        config.actors[i].index = i;
        actorconfig.at("name").get_to(config.actors[i].name);
        actorconfig.at("position").at("x").get_to(config.actors[i].model.position.x);
        actorconfig.at("position").at("y").get_to(config.actors[i].model.position.y);
        actorconfig.at("position").at("z").get_to(config.actors[i].model.position.z);
        actorconfig.at("rotation").at("x").get_to(config.actors[i].model.rotation.x);
        actorconfig.at("rotation").at("y").get_to(config.actors[i].model.rotation.y);
        actorconfig.at("rotation").at("z").get_to(config.actors[i].model.rotation.z);
        actorconfig.at("scale").at("x").get_to(config.actors[i].model.scale.x);
        actorconfig.at("scale").at("y").get_to(config.actors[i].model.scale.y);
        actorconfig.at("scale").at("z").get_to(config.actors[i].model.scale.z);
        actorconfig.at("path").get_to(config.actors[i].model.objPath);
        actorconfig.at("diffuse").get_to(config.actors[i].model.diffusePath);
        actorconfig.at("normal").get_to(config.actors[i].model.normalPath);
        actorconfig.at("roughness").get_to(config.actors[i].model.roughnessPath);
        actorconfig.at("metallic").get_to(config.actors[i].model.metallicPath);
        actorconfig.at("ao").get_to(config.actors[i].model.aoPath);
        actorconfig.at("displacement").get_to(config.actors[i].model.displacementPath);
        actorconfig.at("tesselation").at("level").get_to(config.actors[i].model.tessLevel);
        actorconfig.at("tesselation").at("strength").get_to(config.actors[i].model.tessStregth);
        actorconfig.at("tesselation").at("alpha").get_to(config.actors[i].model.tessAlpha);
        ++i;
    }

    j.at("stage").at("backdrop").get_to(config.stageConfig.backdrop);
    config.stageConfig.models.resize(j["stage"]["models"].size());
    i = 0;
    for (auto &stagemodel : j.at("stage").at("models"))
    {
        config.stageConfig.models[i].index = i;
        stagemodel.at("position").at("x").get_to(config.stageConfig.models[i].position.x);
        stagemodel.at("position").at("y").get_to(config.stageConfig.models[i].position.y);
        stagemodel.at("position").at("z").get_to(config.stageConfig.models[i].position.z);
        stagemodel.at("rotation").at("x").get_to(config.stageConfig.models[i].rotation.x);
        stagemodel.at("rotation").at("y").get_to(config.stageConfig.models[i].rotation.y);
        stagemodel.at("rotation").at("z").get_to(config.stageConfig.models[i].rotation.z);
        stagemodel.at("scale").at("x").get_to(config.stageConfig.models[i].scale.x);
        stagemodel.at("scale").at("y").get_to(config.stageConfig.models[i].scale.y);
        stagemodel.at("scale").at("z").get_to(config.stageConfig.models[i].scale.z);
        stagemodel.at("path").get_to(config.stageConfig.models[i].objPath);
        stagemodel.at("diffuse").get_to(config.stageConfig.models[i].diffusePath);
        stagemodel.at("normal").get_to(config.stageConfig.models[i].normalPath);
        stagemodel.at("roughness").get_to(config.stageConfig.models[i].roughnessPath);
        stagemodel.at("metallic").get_to(config.stageConfig.models[i].metallicPath);
        stagemodel.at("ao").get_to(config.stageConfig.models[i].aoPath);
        stagemodel.at("displacement").get_to(config.stageConfig.models[i].displacementPath);
        stagemodel.at("tesselation").at("level").get_to(config.stageConfig.models[i].tessLevel);
        stagemodel.at("tesselation").at("strength").get_to(config.stageConfig.models[i].tessStregth);
        stagemodel.at("tesselation").at("alpha").get_to(config.stageConfig.models[i].tessAlpha);
        ++i;
    }

    config.pointLights.resize(j["lights"].size());
    i = 0;
    for (auto &lightconfig : j.at("lights"))
    {
        config.pointLights[i].index = i;
        lightconfig.at("name").get_to(config.pointLights[i].name);
        lightconfig.at("position").at("x").get_to(config.pointLights[i].position.x);
        lightconfig.at("position").at("y").get_to(config.pointLights[i].position.y);
        lightconfig.at("position").at("z").get_to(config.pointLights[i].position.z);
        lightconfig.at("temperature").get_to(config.pointLights[i].temperature);
        lightconfig.at("lumens").get_to(config.pointLights[i].lumens);
        ++i;
    }

    j.at("player").at("name").get_to(config.playerConfig.name);
    j.at("player").at("height").get_to(config.playerConfig.height);
    j.at("player").at("mass").get_to(config.playerConfig.mass);
    j.at("player").at("velocityMax").get_to(config.playerConfig.velocityMax);
    j.at("player").at("timeToReachVMax").get_to(config.playerConfig.timeToReachVMax);
    j.at("player").at("timeToStopFromVMax").get_to(config.playerConfig.timeToStopfromVMax);
    j.at("player").at("jumpHeight").get_to(config.playerConfig.jumpHeight);
    j.at("player").at("mouseSensitivity").get_to(config.playerConfig.mouseSensitivity);
    j.at("player").at("fieldOfView").get_to(config.playerConfig.fieldOfView);
    j.at("player").at("position").at("x").get_to(config.playerConfig.position.x);
    j.at("player").at("position").at("y").get_to(config.playerConfig.position.y);
    j.at("player").at("position").at("z").get_to(config.playerConfig.position.z);
    j.at("player").at("rotation").at("x").get_to(config.playerConfig.rotation.x);
    j.at("player").at("rotation").at("y").get_to(config.playerConfig.rotation.y);
    j.at("player").at("rotation").at("z").get_to(config.playerConfig.rotation.z);
};

} // namespace tat