#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include "VulkansEye.h"

const int WIDTH = 800;
const int HEIGHT = 600;

const std::string MODEL_PATH = "resources/models/chalet.obj";
const std::string TEXTURE_PATH = "resources/textures/chalet.jpg";

int main()
{
    VulkansEye app;

    try
    {
        app.init(WIDTH, HEIGHT, TEXTURE_PATH, MODEL_PATH);
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}