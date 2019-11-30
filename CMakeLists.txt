cmake_minimum_required(VERSION 3.0.0)
project(VulkansEye VERSION 0.1.0)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/logs)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wno-pragma-pack -Wno-nonportable-include-path -Wno-documentation -Wno-documentation-unknown-command)

include(CTest)
enable_testing()
include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
${CMAKE_SOURCE_DIR}/src/main.cpp
${CMAKE_SOURCE_DIR}/src/VulkansEye.cpp
${CMAKE_SOURCE_DIR}/src/Config.cpp
${CMAKE_SOURCE_DIR}/src/Camera.cpp
${CMAKE_SOURCE_DIR}/src/Material.cpp
${CMAKE_SOURCE_DIR}/src/Mesh.cpp
${CMAKE_SOURCE_DIR}/src/Object.cpp
${CMAKE_SOURCE_DIR}/src/Model.cpp
${CMAKE_SOURCE_DIR}/src/Backdrop.cpp
${CMAKE_SOURCE_DIR}/src/Player.cpp
${CMAKE_SOURCE_DIR}/src/Scene.cpp
${CMAKE_SOURCE_DIR}/src/engine/Window.cpp
${CMAKE_SOURCE_DIR}/src/engine/Engine.cpp
${CMAKE_SOURCE_DIR}/src/engine/PhysicalDevice.cpp
${CMAKE_SOURCE_DIR}/src/engine/Device.cpp
${CMAKE_SOURCE_DIR}/src/engine/Pipeline.cpp
${CMAKE_SOURCE_DIR}/src/engine/PipelineCache.cpp
${CMAKE_SOURCE_DIR}/src/engine/Buffer.cpp
${CMAKE_SOURCE_DIR}/src/engine/Image.cpp
${CMAKE_SOURCE_DIR}/src/engine/SwapChain.cpp
${CMAKE_SOURCE_DIR}/src/engine/Framebuffer.cpp
${CMAKE_SOURCE_DIR}/src/engine/RenderPass.cpp
${CMAKE_SOURCE_DIR}/src/engine/Allocator.cpp
${CMAKE_SOURCE_DIR}/src/engine/Allocation.cpp
${CMAKE_SOURCE_DIR}/src/engine/Debug.cpp
${CMAKE_SOURCE_DIR}/src/engine/Semaphore.cpp
${CMAKE_SOURCE_DIR}/src/engine/Fence.cpp
${CMAKE_SOURCE_DIR}/src/overlay/Overlay.cpp
${CMAKE_SOURCE_DIR}/src/overlay/Editor.cpp
${CMAKE_SOURCE_DIR}/src/overlay/Info.cpp
${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp 
${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp
)

set(SHADERS 
${CMAKE_SOURCE_DIR}/assets/shaders/shadow.vert
${CMAKE_SOURCE_DIR}/assets/shaders/shadow.frag
${CMAKE_SOURCE_DIR}/assets/shaders/scene.vert
${CMAKE_SOURCE_DIR}/assets/shaders/scene.frag
${CMAKE_SOURCE_DIR}/assets/shaders/backdrop.vert
${CMAKE_SOURCE_DIR}/assets/shaders/backdrop.frag
${CMAKE_SOURCE_DIR}/assets/shaders/ui.vert
${CMAKE_SOURCE_DIR}/assets/shaders/ui.frag
)

set(COMPILED_SHADERS
${CMAKE_SOURCE_DIR}/assets/shaders/shadow.vert.spv
${CMAKE_SOURCE_DIR}/assets/shaders/shadow.frag.spv
${CMAKE_SOURCE_DIR}/assets/shaders/scene.vert.spv
${CMAKE_SOURCE_DIR}/assets/shaders/scene.frag.spv
${CMAKE_SOURCE_DIR}/assets/shaders/backdrop.vert.spv
${CMAKE_SOURCE_DIR}/assets/shaders/backdrop.frag.spv
${CMAKE_SOURCE_DIR}/assets/shaders/ui.vert.spv
${CMAKE_SOURCE_DIR}/assets/shaders/ui.frag.spv
)

foreach(SHADER ${SHADERS})
    add_custom_command(OUTPUT ${SHADER}.spv
        COMMAND glslangValidator -V "${SHADER}" -o "${SHADER}.spv"
        DEPENDS ${SHADER}
        COMMENT "Rebuilding ${SHADER}.spv"
    )
endforeach()

add_executable(VulkansEye
${SOURCES}
${SHADERS}
${COMPILED_SHADERS}
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

find_package(Vulkan REQUIRED)
find_package(assimp CONFIG REQUIRED)

IF(CMAKE_HOST_UNIX)
target_link_libraries(VulkansEye PRIVATE Vulkan::Vulkan glfw ${CMAKE_DL_LIBS} pthread stdc++fs wayland-client)
ENDIF(CMAKE_HOST_UNIX)

IF(CMAKE_HOST_WIN32)
target_compile_definitions(VulkansEye PRIVATE VK_USE_PLATFORM_WIN64_KHR _CRT_SECURE_NO_WARNINGS)
target_link_libraries(VulkansEye PRIVATE glfw Vulkan::Vulkan assimp::assimp)

IF (MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
ENDIF (MSVC)
ENDIF(CMAKE_HOST_WIN32)

target_include_directories(VulkansEye 
    PUBLIC
        "external/glfw/Include"
        "external/glm"
        "external/vma/src"
        "external/gli"
        "external/imgui"
        "external/spdlog/include"
        "external/json/include"
        "external/fmt/include"
        "external/zep/include"
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)