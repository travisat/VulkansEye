#include "VulkansEye.hpp"
#include "Config.hpp"
#include "helpers.hpp"

namespace tat
{

VulkansEye::VulkansEye(const std::string &configPath)
{
    // start timers
    Timer::getInstance();
    Timer::time();
    Timer::systemTime();

    // load config
    Config config = createConfig(configPath);

    // load display settings
    vulkan->name = config.name;
    vulkan->width = config.windowWidth;
    vulkan->height = config.windowHeight;
    vulkan->zNear = config.zNear;
    vulkan->zFar = config.zFar;
    vulkan->brdfPath = config.brdf;
    if (config.sync == true)
    {
        vulkan->defaultPresentMode = vk::PresentModeKHR::eFifo;
    }
    else
    {
        vulkan->defaultPresentMode = vk::PresentModeKHR::eMailbox;
    }

    // setup glfw window
    vulkan->window = std::make_shared<Window>(this, vulkan->width, vulkan->height, "Vulkans Eye");
    
    // setup input
    Input::getInstance();
    vulkan->window->setKeyCallBack(&Input::keyCallback);
    vulkan->window->setMouseButtonCallback(&Input::mouseButtonCallback);
    vulkan->window->setCursorPosCallback(&Input::cursorPosCallback);
    vulkan->window->setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    vulkan->window->setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // create player
    player->vulkan = vulkan;
    player->loadConfig(createPlayerconfig(config.playerConfigPath));

    // create backdrops
    backdrops->vulkan = vulkan;
    backdrops->player = player;
    backdrops->loadConfig(createBackdropsConfig(config.backdropsConfigPath));

    // create materials
    materials->vulkan = vulkan;
    materials->loadConfig(createMaterialsConfig(config.materialsConfigPath));

    // create meshes
    meshes->vulkan = vulkan;
    meshes->loadConfig(createMeshesConfig(config.meshesConfigPath));

    // load scene
    scene->config = createSceneConfig(config.sceneConfigPath);
    scene->vulkan = vulkan;
    scene->player = player;
    scene->backdrops = backdrops;
    scene->materials = materials;
    scene->meshes = meshes;

    // load overlay
    overlay->vulkan = vulkan;
    overlay->player = player;

    // start engine
    engine.vulkan = vulkan;
    engine.scene = scene;
    engine.overlay = overlay;
    engine.init();
}

void VulkansEye::run()
{
    float lastFrameTime = 0.0F;
    while (vulkan->window->shouldClose() == 0)
    {
        float now = Timer::time();
        float deltaTime = now - lastFrameTime;
        lastFrameTime = now;

        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)vulkan->width, (float)vulkan->height);
        io.DeltaTime = deltaTime;
        overlay->newFrame();
        overlay->updateBuffers();

        glfwPollEvents();
        handleInput();

        player->update(deltaTime);

        engine.drawFrame();
    }
    vkDeviceWaitIdle(vulkan->device);
}

void VulkansEye::handleInput()
{
    // game mode
    if (Input::wasKeyReleased(GLFW_KEY_F1))
    {
        if (vulkan->mode != Mode::Game)
        {
            if (displayMode == DisplayMode::cursor)
            {
                vulkan->window->setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                vulkan->window->setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
                player->mouseMode = true;
                displayMode = DisplayMode::nocursor;
            }
            vulkan->showOverlay = false;
            vulkan->updateCommandBuffer = true;
            vulkan->mode = Mode::Game;
        }
    }
    // dbug mode
    if (Input::wasKeyReleased(GLFW_KEY_F2))
    {
        if (vulkan->mode != Mode::Dbug)
        {
            if (displayMode == DisplayMode::cursor)
            {
                vulkan->window->setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                vulkan->window->setInputMode(GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
                player->mouseMode = true;
                displayMode = DisplayMode::nocursor;
            }
            vulkan->showOverlay = true;
            vulkan->updateCommandBuffer = true;
            vulkan->mode = Mode::Dbug;
        }
    }

    // nput mode
    if (Input::wasKeyReleased(GLFW_KEY_F3))
    {
        if (vulkan->mode != Mode::Nput)
        {
            if (displayMode != DisplayMode::cursor)
            {
                vulkan->window->setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                player->mouseMode = false;
                displayMode = DisplayMode::cursor;
            }
            vulkan->mode = Mode::Nput;
        }
    }

    auto moveDir = glm::vec2(0.F);
    moveDir.y += static_cast<float>(Input::isKeyPressed(GLFW_KEY_W));
    moveDir.y -= static_cast<float>(Input::isKeyPressed(GLFW_KEY_S));
    moveDir.x -= static_cast<float>(Input::isKeyPressed(GLFW_KEY_A));
    moveDir.x += static_cast<float>(Input::isKeyPressed(GLFW_KEY_D));
    player->move(moveDir);

    if (Input::isKeyPressed(GLFW_KEY_SPACE) != 0)
    {
        player->jump();
    }

    if (Input::wasKeyReleased(GLFW_KEY_ESCAPE))
    {
        std::cerr << "Pressed Escape.  Closing." << std::endl;
        vulkan->window->setClose(1);
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

auto VulkansEye::createMeshesConfig(const std::string &path) -> MeshesConfig
{
    MeshesConfig config{};
    std::ifstream file(path);
    json j;
    file >> j;

    for (auto &[key, mesh] : j.items())
    {
        MeshConfig c{};
        c.name = key;
        c.path = mesh.value("path", c.path);
        if (mesh.find("center") != mesh.end())
        {
            c.center.x = mesh.at("center").value("x", c.center.x);
            c.center.y = mesh.at("center").value("y", c.center.y);
            c.center.z = mesh.at("center").value("z", c.center.z);
        }
        if (mesh.find("size") != mesh.end())
        {
            c.size.x = mesh.at("size").value("x", c.size.x);
            c.size.y = mesh.at("size").value("y", c.size.y);
            c.size.z = mesh.at("size").value("z", c.size.z);
        }
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