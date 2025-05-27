cmake_minimum_required(VERSION 3.29)
project(wil)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

add_library(${PROJECT_NAME}
	"src/app.cpp"
	"src/log.cpp"
	"src/display.cpp"
	"src/device.cpp"
)

target_include_directories(${PROJECT_NAME} PUBLIC "include")
target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES})

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
