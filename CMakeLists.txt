cmake_minimum_required(VERSION 3.0.0)
project(VulkansEye VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()
include_directories(${CMAKE_SOURCE_DIR}/headers)

set(SHADERS 
${CMAKE_SOURCE_DIR}/resources/shaders/shadow.vert
${CMAKE_SOURCE_DIR}/resources/shaders/shadow.geom
${CMAKE_SOURCE_DIR}/resources/shaders/shadow.frag
${CMAKE_SOURCE_DIR}/resources/shaders/sun.vert
${CMAKE_SOURCE_DIR}/resources/shaders/sun.frag
${CMAKE_SOURCE_DIR}/resources/shaders/scene.vert
${CMAKE_SOURCE_DIR}/resources/shaders/scene.frag
${CMAKE_SOURCE_DIR}/resources/shaders/backdrop.vert
${CMAKE_SOURCE_DIR}/resources/shaders/backdrop.frag
${CMAKE_SOURCE_DIR}/resources/shaders/ui.vert
${CMAKE_SOURCE_DIR}/resources/shaders/ui.frag
)

set(COMPILED_SHADERS
${CMAKE_SOURCE_DIR}/resources/shaders/shadow.vert.spv
${CMAKE_SOURCE_DIR}/resources/shaders/shadow.geom.spv
${CMAKE_SOURCE_DIR}/resources/shaders/shadow.frag.spv
${CMAKE_SOURCE_DIR}/resources/shaders/sun.vert.spv
${CMAKE_SOURCE_DIR}/resources/shaders/sun.frag.spv
${CMAKE_SOURCE_DIR}/resources/shaders/scene.vert.spv
${CMAKE_SOURCE_DIR}/resources/shaders/scene.frag.spv
${CMAKE_SOURCE_DIR}/resources/shaders/backdrop.vert.spv
${CMAKE_SOURCE_DIR}/resources/shaders/backdrop.frag.spv
${CMAKE_SOURCE_DIR}/resources/shaders/ui.vert.spv
${CMAKE_SOURCE_DIR}/resources/shaders/ui.frag.spv
)

foreach(SHADER ${SHADERS})
    add_custom_command(OUTPUT ${SHADER}.spv
        COMMAND glslangValidator -V "${SHADER}" -o "${SHADER}.spv"
        DEPENDS ${SHADER}
        COMMENT "Rebuilding ${SHADER}.spv"
    )
endforeach()

set(SOURCES
${CMAKE_SOURCE_DIR}/src/main.cpp
${CMAKE_SOURCE_DIR}/src/VulkansEye.cpp
${CMAKE_SOURCE_DIR}/src/Engine.cpp
${CMAKE_SOURCE_DIR}/src/Pipeline.cpp
${CMAKE_SOURCE_DIR}/src/Framebuffer.cpp
${CMAKE_SOURCE_DIR}/src/RenderPass.cpp
${CMAKE_SOURCE_DIR}/src/Buffer.cpp
${CMAKE_SOURCE_DIR}/src/Image.cpp
${CMAKE_SOURCE_DIR}/src/Materials.cpp
${CMAKE_SOURCE_DIR}/src/Meshes.cpp
${CMAKE_SOURCE_DIR}/src/Model.cpp
${CMAKE_SOURCE_DIR}/src/Backdrop.cpp
${CMAKE_SOURCE_DIR}/src/Player.cpp
${CMAKE_SOURCE_DIR}/src/PointLight.cpp
${CMAKE_SOURCE_DIR}/src/Scene.cpp
${CMAKE_SOURCE_DIR}/src/Overlay.cpp
${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp 
${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp
)

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

IF(CMAKE_HOST_UNIX)
target_link_libraries(VulkansEye Vulkan::Vulkan glfw ${CMAKE_DL_LIBS} pthread stdc++fs wayland-client)
ENDIF(CMAKE_HOST_UNIX)

IF(CMAKE_HOST_WIN32)
target_compile_definitions(VulkansEye PRIVATE VK_USE_PLATFORM_WIN64_KHR _CRT_SECURE_NO_WARNINGS)
target_link_libraries(VulkansEye glfw Vulkan::Vulkan)

IF (MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
ENDIF (MSVC)
ENDIF(CMAKE_HOST_WIN32)

target_include_directories(VulkansEye 
    PUBLIC 
        "external/glfw/Include"
        "external/GSL/Include"
        "external/glm"
        "external/tinygltf"
        "external/tinyobjloader"
        "external/vma/src"
        "external/gli"
        "external/imgui"
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
