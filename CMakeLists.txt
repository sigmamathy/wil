cmake_minimum_required(VERSION 3.20)
project(wil)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

add_library(${PROJECT_NAME}

	"deps/imgui/src/imgui.cpp"
	"deps/imgui/src/imgui_demo.cpp"
	"deps/imgui/src/imgui_draw.cpp"
	"deps/imgui/src/imgui_tables.cpp"
	"deps/imgui/src/imgui_widgets.cpp"
	"deps/imgui/src/imgui_impl_glfw.cpp"
	"deps/imgui/src/imgui_impl_vulkan.cpp"
	"deps/stb/src/stb_image.c"
	"deps/stb/src/stb_image_write.c"
	"deps/stb/src/stb_truetype.c"
	"deps/tinygltf/src/tiny_gltf.cc"

	"src/app.cpp"
	"src/log.cpp"
	"src/display.cpp"
	"src/device.cpp"
	"src/pipeline.cpp"
	"src/drawsync.cpp"
	"src/buffer.cpp"
	"src/scene.cpp"
	"src/transform.cpp"
	"src/descriptor.cpp"
	"src/model.cpp"
	"src/cmdbuf.cpp"
	"src/render.cpp"
)

target_include_directories(${PROJECT_NAME} PUBLIC "include" "deps/stb/include" "deps/tinygltf/include" "deps/imgui/include" ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES})

set(WIL_SHADER_SRC_DIRECTORY "${PROJECT_SOURCE_DIR}/shaders")
set(WIL_RES_DIRECTORY "${PROJECT_SOURCE_DIR}/res")

# List of all shaders
set(SHADER_SOURCES
    "3d.vert"
    "3d.frag"
	"light3d.vert"
	"light3d.frag"
)

if (NOT EXISTS "${CMAKE_BINARY_DIR}/shaders")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")
endif()

file(MAKE_DIRECTORY "${WIL_RES_DIRECTORY}/wil/shaders")

# Compile all shaders using glslc upon each build
set(SHADER_TARGET_INDEX 0)
foreach(SHADER ${SHADER_SOURCES})
	add_custom_command(OUTPUT "${WIL_RES_DIRECTORY}/wil/shaders/${SHADER}.spv"
		COMMAND glslc "${WIL_SHADER_SRC_DIRECTORY}/${SHADER}" -o "${WIL_RES_DIRECTORY}/wil/shaders/${SHADER}.spv"
		DEPENDS "${WIL_SHADER_SRC_DIRECTORY}/${SHADER}"
        COMMENT "Compile shader module ${SHADER}"
	)
	add_custom_target("shader_build_${SHADER_TARGET_INDEX}" ALL
		DEPENDS "${WIL_RES_DIRECTORY}/wil/shaders/${SHADER}.spv"
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
