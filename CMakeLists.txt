cmake_minimum_required(VERSION 3.0.0)
project(VulkansEye VERSION 0.1.0)

include(CTest)
enable_testing()
include_directories(${CMAKE_SOURCE_DIR}/headers)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
pkg_search_module(VULKAN REQUIRED vulkan)

set(SHADERS 
${CMAKE_SOURCE_DIR}/resources/shaders/shadow.vert
${CMAKE_SOURCE_DIR}/resources/shaders/shadow.geom
${CMAKE_SOURCE_DIR}/resources/shaders/shadow.frag
${CMAKE_SOURCE_DIR}/resources/shaders/scene.vert
${CMAKE_SOURCE_DIR}/resources/shaders/scene.tesc
${CMAKE_SOURCE_DIR}/resources/shaders/scene.tese
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
${CMAKE_SOURCE_DIR}/resources/shaders/scene.vert.spv
${CMAKE_SOURCE_DIR}/resources/shaders/scene.tesc.spv
${CMAKE_SOURCE_DIR}/resources/shaders/scene.tese.spv
${CMAKE_SOURCE_DIR}/resources/shaders/scene.frag.spv
${CMAKE_SOURCE_DIR}/resources/shaders/backdrop.vert.spv
${CMAKE_SOURCE_DIR}/resources/shaders/backdrop.frag.spv
${CMAKE_SOURCE_DIR}/resources/shaders/ui.vert.spv
${CMAKE_SOURCE_DIR}/resources/shaders/ui.frag.spv
)

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

set_target_properties(VulkansEye 
    PROPERTIES
        CXX_STANDARD 17
)   

foreach(SHADER ${SHADERS})
    add_custom_command(OUTPUT ${SHADER}.spv
        COMMAND glslangValidator -V "${SHADER}" -o "${SHADER}.spv"
        DEPENDS ${SHADER}
        COMMENT "Rebuilding ${SHADER}.spv"
    )
endforeach()

target_include_directories(VulkansEye 
    PUBLIC 
        "${GLFW_INCLUDE_DIRS}"
        "${VULKAN_INCLUDE_DIRS}"
        "external/GSL/Include"
        "external/glm"
        "external/tinygltf"
        "external/tinyobjloader"
        "external/vma/src"
        "external/gli"
        "external/imgui"
)

if (CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(VulkansEye ${VULKAN_LIBRARIES} ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS} pthread stdc++fs wayland-client)
endif (CMAKE_COMPILER_IS_GNUCXX)

# Compiler specific stuff
IF (MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
ENDIF (MSVC)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
