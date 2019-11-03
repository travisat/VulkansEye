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
    Config config = createConfig(configPath);

    // load display settings
    vulkan.name = config.name;
    vulkan.width = config.windowWidth;
    vulkan.height = config.windowHeight;
    vulkan.zNear = config.zNear;
    vulkan.zFar = config.zFar;
    vulkan.brdfPath = config.brdf;
    if (config.sync == true)
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
    player.vulkan = &vulkan;
    player.loadConfig(createPlayerconfig(config.playerConfigPath));

    // create backdrops
    backdrops.vulkan = &vulkan;
    backdrops.player = &player;
    backdrops.loadConfig(createBackdropsConfig(config.backdropsConfigPath));

    // create materials
    materials.vulkan = &vulkan;
    materials.loadConfig(createMaterialsConfig(config.materialsConfigPath));

    // create meshes
    meshes.vulkan = &vulkan;
    meshes.loadConfig(createMeshesConfig(config.meshesConfigPath));

    // load scene
    scene.config = createSceneConfig(config.sceneConfigPath);
    scene.vulkan = &vulkan;
    scene.player = &player;
    scene.backdrops = &backdrops;
    scene.materials = &materials;
    scene.meshes = &meshes;

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

auto VulkansEye::createConfig(const std::string &path) -> Config
{
    Config config;
    std::ifstream file(path);
    json j;
    file >> j;

    config.name = j.value("name", config.name);
    config.zNear = j.value("zNear", config.zNear);
    config.zFar = j.value("zFar", config.zFar);
    config.windowWidth = j.value("windowWidth", config.windowWidth);
    config.windowHeight = j.value("windowHeight", config.windowHeight);
    config.sync = j.value("sync", config.sync);
    config.shadowSize = j.value("shadowSize", config.shadowSize);
    config.brdf = j.value("brdfPath", config.brdf);
    config.playerConfigPath = j.value("playerConfigPath", config.playerConfigPath);
    config.backdropsConfigPath = j.value("backdropsConfigPath", config.backdropsConfigPath);
    config.materialsConfigPath = j.value("materialsConfigPath", config.materialsConfigPath);
    config.meshesConfigPath = j.value("meshesConfigPath", config.meshesConfigPath);
    config.sceneConfigPath = j.value("sceneConfigPath", config.sceneConfigPath);
    return config;
}

auto VulkansEye::createPlayerconfig(const std::string &path) -> PlayerConfig
{
    PlayerConfig config{};
    std::ifstream file(path);
    json j;
    file >> j;

    config.height = j.value("height", config.height);
    config.mass = j.value("mass", config.mass);
    config.velocityMax = j.value("velocityMax", config.velocityMax);
    config.timeToReachVMax = j.value("timeToReachVMax", config.timeToReachVMax);
    config.timeToStopfromVMax = j.value("timeToStopFromVMax", config.timeToStopfromVMax);
    config.jumpHeight = j.value("jumpHeight", config.jumpHeight);
    config.mouseSensitivity = j.value("mouseSensitivity", config.mouseSensitivity);
    config.fieldOfView = j.value("fieldOfView", config.fieldOfView);

    return config;
}

auto VulkansEye::createBackdropsConfig(const std::string &path) -> BackdropsConfig
{
    BackdropsConfig config{};
    std::ifstream file(path);
    json j;
    file >> j;
    for (auto &[key, backdrop] : j.items())
    {
        BackdropConfig c{};
        c.name = key;
        c.colorPath = backdrop.value("color", c.colorPath);
        c.radiancePath = backdrop.value("radiance", c.radiancePath);
        c.irradiancePath = backdrop.value("irradiance", c.irradiancePath);
        if (backdrop.find("light") != backdrop.end())
        {
            auto light = backdrop.at("light");
            if (light.find("position") != light.end())
            {
                auto position = light.at("position");
                c.light.position.x = position.value("x", c.light.position.x);
                c.light.position.y = position.value("y", c.light.position.y);
                c.light.position.z = position.value("z", c.light.position.z);
            }
            c.light.steradians = light.value("steradians", c.light.steradians);
            c.light.temperature = light.value("temperature", c.light.temperature);
            c.light.lumens = light.value("lumens", c.light.lumens);
        }
        config.backdrops.push_back(c);
    }
    return config;
}

auto VulkansEye::createMaterialsConfig(const std::string &path) -> MaterialsConfig
{
    MaterialsConfig config{};
    std::ifstream file(path);
    json j;
    file >> j;

    for (auto &[key, material] : j.items())
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
    return config;
}

auto VulkansEye::createMeshesConfig(const std::string &path) ->  MeshesConfig
{
    MeshesConfig config{};
    std::ifstream file(path);
    json j;
    file >> j;

    for (auto &[key, value] : j.items())
    {
        MeshConfig c{};
        c.name = key;
        c.path = value;
        config.meshes.push_back(c);
    }
    return config;
}

 auto VulkansEye::createSceneConfig(const std::string &path) -> SceneConfig
{
    SceneConfig config{};
    std::ifstream file(path);
    json j;
    file >> j;

    config.name = j.value("name", config.name);
    config.backdrop = j.value("backdrop", config.backdrop);

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
    return config;
}

} // namespace tat