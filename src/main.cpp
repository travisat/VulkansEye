#include <memory>
#include <system_error>
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include <filesystem>
#include <iostream>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include "VulkansEye.hpp"

#define DEFAULT_CONFIG "assets/configs/settings.json"

auto main(int argc, char *argv[]) -> int
{

    try
    {
        auto debugLogger = spdlog::basic_logger_mt<spdlog::async_factory>("debugLogger", "logs/debug.log", true);
        spdlog::set_default_logger(debugLogger);
        debugLogger->info("BEGIN");

        try
        {
            std::string config = DEFAULT_CONFIG;
            switch (argc)
            {
            case 0:
            case 1:
                break;
            case 2:
                if (std::filesystem::exists(argv[1]))
                {
                    config = argv[1];
                    break;
                } // fall through if doesn't exist
            default:
                std::cout << "Usage: VulkansEye [config]" << std::endl;
                return EXIT_FAILURE;
            }

            tat::VulkansEye app(config);
            app.run();
        }
        catch (const std::exception &e)
        {
            debugLogger->error("Error {}", e.what());
            debugLogger->error("Stopping");
            return EXIT_FAILURE;
        }

        debugLogger->info("END");
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}