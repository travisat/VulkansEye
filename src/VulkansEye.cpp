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

    ModelConfig stageModel;    
    stageModel.modelType = ModelType::obj;
    stageModel.objPath = "resources/stage/floor.obj";
    stageModel.imageType = ImageType::png;
    stageModel.diffusePath = "resources/textures/brick/diffuse.png";
    stageModel.normalPath = "resources/textures/brick/normal.png";
    stageModel.roughnessPath = "resources/textures/brick/roughness.png";
    stageModel.ambientOcclusionPath = "resources/textures/brick/ambientOcclusion.png";

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

    const float p = 15.0f;
    LightConfig light0;
    light0.name = "light0";
    light0.index = 0;
    light0.position = {-p, -p * 0.5f, -p};
    LightConfig light1;
    light1.name = "light1";
    light1.index = 1;
    light1.position = {-p, -p * 0.5f, p};
    config.lights = {light0, light1};

    ModelConfig tableModel;
    tableModel.index = 0;
    tableModel.name = "Table";
    tableModel.imageType = ImageType::png;
    tableModel.diffusePath = "resources/textures/alien/diffuse.png";
    tableModel.normalPath = "resources/textures/alien/normal.png";
    tableModel.roughnessPath = "resources/textures/alien/roughness.png";
    tableModel.ambientOcclusionPath = "resources/textures/alien/ambientOcclusion.png";
    tableModel.modelType = ModelType::obj;
    tableModel.objPath = "resources/models/table.obj";

    ActorConfig table;
    table.index = 0;
    table.name = "table Model";
    table.position = glm::vec3(4.0f, 0.0f, 5.0f);
    table.scale = glm::vec3(0.25f);
    table.modelConfig = tableModel;

    config.actors = {table};

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
