cmake_minimum_required(VERSION 3.0.0)
project(VulkansEye VERSION 0.1.0)

include(CTest)
enable_testing()
include_directories(${CMAKE_SOURCE_DIR}/headers)

set(VULKAN 
C:/VulkanSDK/1.1.114.0
)

set(GLFW 
${CMAKE_SOURCE_DIR}/../../Libraries/glfw-3.3.bin.WIN64
)

set(SHADERS 
${CMAKE_SOURCE_DIR}/resources/shaders/scene.vert
${CMAKE_SOURCE_DIR}/resources/shaders/scene.frag
${CMAKE_SOURCE_DIR}/resources/shaders/skybox.vert
${CMAKE_SOURCE_DIR}/resources/shaders/skybox.frag
${CMAKE_SOURCE_DIR}/resources/shaders/ui.vert
${CMAKE_SOURCE_DIR}/resources/shaders/ui.frag
)

set(COMPILED_SHADERS
${CMAKE_SOURCE_DIR}/resources/shaders/scene.vert.spv
${CMAKE_SOURCE_DIR}/resources/shaders/scene.frag.spv
${CMAKE_SOURCE_DIR}/resources/shaders/skybox.vert.spv
${CMAKE_SOURCE_DIR}/resources/shaders/skybox.frag.spv
${CMAKE_SOURCE_DIR}/resources/shaders/ui.vert.spv
${CMAKE_SOURCE_DIR}/resources/shaders/ui.frag.spv
)

set(SOURCES
${CMAKE_SOURCE_DIR}/src/main.cpp
${CMAKE_SOURCE_DIR}/src/VulkansEye.cpp
${CMAKE_SOURCE_DIR}/src/VkEngine.cpp
${CMAKE_SOURCE_DIR}/src/Buffer.cpp
${CMAKE_SOURCE_DIR}/src/Model.cpp
${CMAKE_SOURCE_DIR}/src/Actor.cpp
${CMAKE_SOURCE_DIR}/src/Image.cpp
${CMAKE_SOURCE_DIR}/src/Backdrop.cpp
${CMAKE_SOURCE_DIR}/src/Player.cpp
${CMAKE_SOURCE_DIR}/src/PointLight.cpp
${CMAKE_SOURCE_DIR}/src/Scene.cpp
${CMAKE_SOURCE_DIR}/src/Stage.cpp
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
        COMMAND glslangvalidator -V "${SHADER}" -o "${SHADER}.spv"
        DEPENDS ${SHADER}
        COMMENT "Rebuilding ${SHADER}.spv"
    )
    message(STATUS "Generating build commands for ${SHADER}.spv")  
endforeach()

target_include_directories(VulkansEye 
    PUBLIC 
        "${GLFW}/include"
        "${VULKAN}/Include"
        "external/GSL/Include"
        "external/glm"
        "external/tinygltf"
        "external/tinyobjloader"
        "external/vma/src"
        "external/gli"
        "external/imgui"
)

add_library(vulkan STATIC IMPORTED)
set_property(TARGET vulkan PROPERTY IMPORTED_LOCATION "${VULKAN}/lib/vulkan-1.lib")
target_compile_definitions(VulkansEye PRIVATE VK_USE_PLATFORM_WIN64_KHR)
target_link_libraries(VulkansEye vulkan)

add_library(glfw STATIC IMPORTED)
set_property(TARGET glfw PROPERTY IMPORTED_LOCATION "${GLFW}/lib-vc2019/glfw3.lib")
target_link_libraries(VulkansEye glfw)

# Compiler specific stuff
IF(MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
ENDIF(MSVC)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
