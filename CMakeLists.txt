cmake_minimum_required(VERSION 3.0.0)
project(VulkansEye VERSION 0.1.0)

include(CTest)
enable_testing()
include_directories(${CMAKE_SOURCE_DIR}/headers)

set(VULKAN 
C:/VulkanSDK/1.1.114.0
)

set(SHADERS 
${CMAKE_SOURCE_DIR}/resources/shaders/scene.vert
${CMAKE_SOURCE_DIR}/resources/shaders/scene.frag
${CMAKE_SOURCE_DIR}/resources/shaders/skybox.vert
${CMAKE_SOURCE_DIR}/resources/shaders/skybox.frag
)

set(COMPILED_SHADERS
${CMAKE_SOURCE_DIR}/resources/shaders/scene.vert.spv
${CMAKE_SOURCE_DIR}/resources/shaders/scene.frag.spv
${CMAKE_SOURCE_DIR}/resources/shaders/skybox.vert.spv
${CMAKE_SOURCE_DIR}/resources/shaders/skybox.frag.spv
)

set(SOURCES
${CMAKE_SOURCE_DIR}/src/main.cpp
${CMAKE_SOURCE_DIR}/src/VulkansEye.cpp
${CMAKE_SOURCE_DIR}/src/VkEngine.cpp
${CMAKE_SOURCE_DIR}/src/State.cpp
${CMAKE_SOURCE_DIR}/src/Buffer.cpp
${CMAKE_SOURCE_DIR}/src/Mesh.cpp
${CMAKE_SOURCE_DIR}/src/Material.cpp
${CMAKE_SOURCE_DIR}/src/Model.cpp
${CMAKE_SOURCE_DIR}/src/Image.cpp
${CMAKE_SOURCE_DIR}/src/Skybox.cpp
${CMAKE_SOURCE_DIR}/src/Camera.cpp
${CMAKE_SOURCE_DIR}/src/Light.cpp
${CMAKE_SOURCE_DIR}/src/Scene.cpp
)

add_executable(VulkansEye
${SOURCES}
${SHADERS}
${COMPILED_SHADERS}
)

foreach(SHADER ${SHADERS})
add_custom_command(OUTPUT ${SHADER}.spv
COMMAND glslangvalidator -V "${SHADER}" -o "${SHADER}.spv"
DEPENDS ${SHADER}
COMMENT "Rebuilding ${SHADER}.spv"
)
message(STATUS "Generating build commands for ${SHADER}.spv")  
endforeach()

set_target_properties(VulkansEye PROPERTIES
            CXX_STANDARD 17)   

set(myLibraries "${CMAKE_SOURCE_DIR}/../../Libraries")

target_include_directories(VulkansEye 
    PUBLIC 
        "${myLibraries}/glfw-3.3.bin.WIN64/include"
        "${myLibraries}/glm"
        "${myLibraries}/tinygltf-2.2.0"
        "${myLibraries}/tinyobjloader"
        "${myLibraries}/vma-2.2.0/src"
        "${myLibraries}/gli"
        "${VULKAN}/Include"
        )

add_library(vulkan STATIC IMPORTED)
set_property(TARGET vulkan PROPERTY IMPORTED_LOCATION "${VULKAN}/lib/vulkan-1.lib")
target_compile_definitions(VulkansEye PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_link_libraries(VulkansEye vulkan)

add_library(glfw STATIC IMPORTED)
set_property(TARGET glfw PROPERTY IMPORTED_LOCATION "${myLibraries}/glfw-3.3.bin.WIN64/lib-vc2019/glfw3.lib")
target_link_libraries(VulkansEye glfw)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
