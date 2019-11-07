#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include <filesystem>

#include "VulkansEye.hpp"

#define DEFAULT_CONFIG "assets/configs/settings.json"

auto main(int argc, char *argv[]) -> int
{
#ifdef WIN32
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif

    tat::VulkansEye app;
    try
    {
        switch (argc)
        {
        case 0:
        case 1:
            app.init(DEFAULT_CONFIG);
            break;
        case 2:
            if (std::filesystem::exists(argv[1]))
            {
                app.init(argv[1]);
                break;
            } // fall through if doesn't exist
        default:
            std::cerr << "Usage: VulkansEye [config]" << std::endl;
            return EXIT_FAILURE;
        }
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

#ifdef WIN32
    CoUninitialize();
#endif

    return EXIT_SUCCESS;
}