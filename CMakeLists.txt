cmake_minimum_required(VERSION 3.0.0)
project(VulkanTest VERSION 0.1.0)

include(CTest)
enable_testing()
include_directories(headers)

add_executable(VulkanTest src/main.cpp src/HelloWorldTriangle.cpp)
set_target_properties(VulkanTest PROPERTIES
            CXX_STANDARD 17)   

set(myLibraries "C:/Users/travi_000/Documents/Visual Studio Code/Libraries")

target_include_directories(VulkanTest 
    PUBLIC 
        "${myLibraries}/glfw-3.3.bin.WIN64/include"
        "${myLibraries}/glm"
        "${myLibraries}/stb"
        "${myLibraries}/tinyobjloader"
        "C:/VulkanSDK/1.1.114.0/Include"
        )

add_library(vulkan STATIC IMPORTED)
set_property(TARGET vulkan PROPERTY IMPORTED_LOCATION "C:/VulkanSDK/1.1.114.0/lib/vulkan-1.lib")
target_compile_definitions(VulkanTest PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_link_libraries(VulkanTest vulkan)

add_library(glfw STATIC IMPORTED)
set_property(TARGET glfw PROPERTY IMPORTED_LOCATION "C:/Users/travi_000/Documents/Visual Studio Code/Libraries/glfw-3.3.bin.WIN64/lib-vc2019/glfw3.lib")
target_link_libraries(VulkanTest glfw)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
