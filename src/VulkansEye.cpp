#include "VulkansEye.hpp"
#include "helpers.h"

void VulkansEye::init(uint32_t width, uint32_t height)
{
    vulkan.name = "Vulkans Eye";
    vulkan.width = width;
    vulkan.height = height;

    //setup timers
    Timer::getInstance();
    Timer::systemTime();

    //setup glfw window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    vulkan.window = glfwCreateWindow(vulkan.width, vulkan.height, "Vulkans Eye", nullptr, nullptr);
    glfwSetWindowUserPointer(vulkan.window, this);
    glfwSetFramebufferSizeCallback(vulkan.window, framebufferResizeCallback);

    //setup input
    Input::getInstance();
    glfwSetKeyCallback(vulkan.window, &Input::keyCallback);
    glfwSetMouseButtonCallback(vulkan.window, &Input::mouseButtonCallback);
    glfwSetCursorPosCallback(vulkan.window, &Input::cursorPosCallback);

    //setup config
    //TODO load from config file
    SceneConfig config;
    config.backdrop = {"resources/backdrop/nebula.dds"};


    MaterialConfig pavingStones;
    pavingStones.imageType = ImageType::jpg;
    pavingStones.diffusePath = "resources/textures/pavingstones/PavingStones45_col.jpg";
    pavingStones.normalPath = "resources/textures/pavingstones/PavingStones45_nrm.jpg";
    pavingStones.roughnessPath = "resources/textures/pavingstones/PavingStones45_rgh.jpg";
    pavingStones.aoPath = "resources/textures/pavingstones/PavingStones45_AO.jpg";

    MaterialConfig wood16;
    wood16.imageType = ImageType::jpg;
    wood16.diffusePath = "resources/textures/wood16/diffuse.jpg";
    wood16.normalPath = "resources/textures/wood16/normal.jpg";
    wood16.roughnessPath = "resources/textures/wood16/roughness.jpg";

    MaterialConfig metal06;
    metal06.imageType = ImageType::jpg;
    metal06.diffusePath = "resources/textures/metal06/diffuse.jpg";
    metal06.normalPath = "resources/textures/metal06/normal.jpg";
    metal06.roughnessPath = "resources/textures/metal06/roughness.jpg";
    metal06.metallicPath = "resources/textures/metal06/metallic.jpg";

    ModelConfig stageModel;    
    stageModel.modelType = ModelType::obj;
    stageModel.objPath = "resources/stage/floor.obj";
    stageModel.material = pavingStones;

    StageConfig stageConfig;
    stageConfig.index = 0;
    stageConfig.name = "floor";
    stageConfig.modelConfig = stageModel;
    config.stageConfig = stageConfig;
    
    PlayerConfig playerConfig;
    playerConfig.name = "You";
    playerConfig.height = 1.7f;
    playerConfig.fieldOfView = 60.0f;
    playerConfig.position = {0.0f, -playerConfig.height, 0.0f};
    playerConfig.rotation = {0.0f, 0.0f, 0.0f};
    
    config.playerConfig = playerConfig;

    LightConfig light0;
    light0.name = "light0";
    light0.index = 0;
    light0.position = {4.0f, 2.4f, 5.0f};
    light0.temperature = 4000.0f;
    light0.lumens = 800.0f;
    LightConfig light1;
    light1.name = "lantern";
    light1.index = 1;
    light1.position = {0.0f, 2.7f, 0.0f};
    light1.temperature = 2500.0f;
    light1.lumens = 400.0f;
    config.lights = {light0};

    ModelConfig tableModel;
    tableModel.index = 0;
    tableModel.name = "Table";
    tableModel.modelType = ModelType::obj;
    tableModel.objPath = "resources/models/table.obj";
    tableModel.material = wood16;

    ModelConfig lightSphere;
    lightSphere.index = 0;
    lightSphere.name = "sphere";
    lightSphere.modelType = ModelType::obj;
    lightSphere.objPath = "resources/models/sphere.obj";
    lightSphere.material = metal06;

    ActorConfig light;
    light.index = 1;
    light.name = "globe light";
    light.position = {4.0f, 1.0f, 5.0f};
    light.scale = glm::vec3(0.4f);
    light.modelConfig = lightSphere;

    ActorConfig table;
    table.index = 0;
    table.name = "table Model";
    table.position = glm::vec3(4.0f, 0.0f, 5.0f);
    table.scale = glm::vec3(1.0f);
    table.modelConfig = tableModel;

    config.actors = {table, light};

    //setup player
    player.height = config.playerConfig.height;
    player.setPerspective(config.playerConfig.fieldOfView, static_cast<double>(vulkan.width), static_cast<double>(vulkan.height), 0.1f, 512.0f);
    player.position = config.playerConfig.position;
    player.rotation = config.playerConfig.rotation;
    player.updateView();

    //setup scene
    scene.config = &config;
    scene.vulkan = &vulkan;
    scene.player = &player;
    scene.name = "Scene";

    //setup overlay
    overlay.vulkan = &vulkan;
    overlay.player = &player;

    //setup engine
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

void VulkansEye::mainLoop()
{
    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(vulkan.window))
    {
        auto deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(Timer::getTime() - lastFrameTime).count();
        lastFrameTime = Timer::getTime();

        glfwPollEvents();

        if (Input::checkMouse(GLFW_MOUSE_BUTTON_2))
        {
            if (!player.mouseMode)
            {
                glfwSetInputMode(vulkan.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetInputMode(vulkan.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
                player.mouseMode = true;
            }
        }   
        else
        {
            glfwSetInputMode(vulkan.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            player.mouseMode = false;
        }

        player.update(deltaTime); 

        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)vulkan.width, (float)vulkan.height);
        io.DeltaTime = deltaTime;

        overlay.newFrame();

        overlay.updateBuffers();

        engine.drawFrame();
        vulkan.frameCounter++;

        if (Input::checkKeyboard(GLFW_KEY_ESCAPE))
        {
            std::cerr << "Pressed Escape.  Closing." << std::endl;
            glfwSetWindowShouldClose(vulkan.window, true);
        }
    }
    vkDeviceWaitIdle(vulkan.device);
}

void VulkansEye::cleanup()
{
    glfwDestroyWindow(vulkan.window);
    glfwTerminate();
}
