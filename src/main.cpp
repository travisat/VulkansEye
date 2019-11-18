#include <memory>
#include <system_error>
#include <filesystem>
#include <iostream>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include "VulkansEye.hpp"

constexpr auto defaultConfig = "assets/configs/settings.json";

auto main(int argc, char *argv[]) -> int
{
    try
    {
        auto debug = spdlog::basic_logger_mt<spdlog::async_factory>("debug", "logs/debug.log", true);
        auto state = spdlog::basic_logger_mt<spdlog::async_factory>("state", "logs/state.log", true);
        spdlog::set_default_logger(debug);
        spdlog::info("BEGIN");

        try
        {
            std::string config = defaultConfig;
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
            tat::VulkansEye::run();
            tat::VulkansEye::cleanup();
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error {}", e.what());
            spdlog::error("Stopping");
            return EXIT_FAILURE;
        }
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}