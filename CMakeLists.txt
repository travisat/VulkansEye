cmake_minimum_required(VERSION 3.0.0)
project(VulkansEye VERSION 0.1.0)

include(CTest)
enable_testing()
include_directories(headers)

add_executable(VulkansEye
                src/main.cpp
                src/VulkansEye.cpp
                src/VkBackend.cpp
                src/Buffer.cpp
                src/Mesh.cpp
                src/Material.cpp
                src/Model.cpp
                src/Image.cpp
                src/State.cpp
                src/Skybox.cpp
                src/Camera.cpp
                src/Light.cpp
                src/Scene.cpp)

set_target_properties(VulkansEye PROPERTIES
            CXX_STANDARD 17)   

set(myLibraries "C:/Users/travi_000/Documents/Visual Studio Code/Libraries")

target_include_directories(VulkansEye 
    PUBLIC 
        "${myLibraries}/glfw-3.3.bin.WIN64/include"
        "${myLibraries}/glm"
        "${myLibraries}/tinygltf-2.2.0"
        "${myLibraries}/tinyobjloader"
        "${myLibraries}/vma-2.2.0/src"
        "${myLibraries}/gli"
        "C:/VulkanSDK/1.1.114.0/Include"
        )

add_library(vulkan STATIC IMPORTED)
set_property(TARGET vulkan PROPERTY IMPORTED_LOCATION "C:/VulkanSDK/1.1.114.0/lib/vulkan-1.lib")
target_compile_definitions(VulkansEye PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_link_libraries(VulkansEye vulkan)

add_library(glfw STATIC IMPORTED)
set_property(TARGET glfw PROPERTY IMPORTED_LOCATION "C:/Users/travi_000/Documents/Visual Studio Code/Libraries/glfw-3.3.bin.WIN64/lib-vc2019/glfw3.lib")
target_link_libraries(VulkansEye glfw)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
