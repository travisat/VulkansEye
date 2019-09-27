#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include "VulkansEye.h"

const int WIDTH = 800;
const int HEIGHT = 600;

int main()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    VulkansEye app;
    try
    {
        app.init(WIDTH, HEIGHT);
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    CoUninitialize();
    return EXIT_SUCCESS;
}