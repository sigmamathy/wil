cmake_minimum_required(VERSION 3.29)
project(wil)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

add_library(${PROJECT_NAME}
	"deps/stb/src/stb_image.c"
	"src/app.cpp"
	"src/log.cpp"
	"src/display.cpp"
	"src/device.cpp"
	"src/pipeline.cpp"
	"src/layer.cpp"
	"src/drawsync.cpp"
	"src/buffer.cpp"
	"src/scene.cpp"
	"src/transform.cpp"
)

target_include_directories(${PROJECT_NAME} PUBLIC "include" "deps/stb/include")
target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES})

# List of all shaders
set(SHADER_SOURCES
    "3d.vert"
    "3d.frag"
)

if (NOT EXISTS "${CMAKE_BINARY_DIR}/shaders")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")
endif()

# Compile all shaders using glslc upon each build
set(SHADER_TARGET_INDEX 0)
foreach(SHADER ${SHADER_SOURCES})
    add_custom_target("shader_build_${SHADER_TARGET_INDEX}"
            COMMAND glslc "${PROJECT_SOURCE_DIR}/shaders/${SHADER}" -o "${CMAKE_BINARY_DIR}/shaders/${SHADER}.spv"
            COMMENT "Compile shader module ${SHADER}"
    )
    add_dependencies(${PROJECT_NAME} "shader_build_${SHADER_TARGET_INDEX}")
    math(EXPR SHADER_TARGET_INDEX "${SHADER_TARGET_INDEX} + 1")
endforeach()

if (PROJECT_IS_TOP_LEVEL AND UNIX)
    # Create symlink to compile_commands.json for IDE to pick it up
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    )
endif()

option(WIL_BUILD_TESTS "Build tests" OFF)

if (WIL_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()
