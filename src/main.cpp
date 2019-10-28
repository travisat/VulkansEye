#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include "VulkansEye.hpp"

auto main() -> int
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    tat::VulkansEye app;
    try
    {
        app.init();
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