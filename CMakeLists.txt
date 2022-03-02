cmake_minimum_required(VERSION 3.20)
project(vulkan_engine)

set(CMAKE_CXX_STANDARD 17)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/lua)
add_subdirectory(thirdparty/imgui)

# Vulkan
find_package(Vulkan REQUIRED)

# OpenGL
find_package(OpenGL REQUIRED)

# Messages
message("Libraries:")
message(VULKAN_INCLUDE_DIRS: ${Vulkan_INCLUDE_DIRS})
message(VULKAN_LIB: ${Vulkan_LIBRARIES})
message(OPENGL_LIBRARIES: ${OPENGL_LIBRARIES})

# include directories
include_directories(
        thirdparty
)

# load source and header files
file(GLOB_RECURSE SOURCE_FILES
        "src/*.h"
        "src/*.cpp")

add_executable(${PROJECT_NAME} WIN32
        ${SOURCE_FILES}
        thirdparty/glad/glad.c
        )

if(WIN32)
    target_link_libraries(
            ${PROJECT_NAME}
            glfw
            Vulkan::Vulkan
            d3d12
            OpenGL::GL
            lua_lib
            imgui
    )
else()
    target_link_libraries(
            ${PROJECT_NAME}
            glfw
            Vulkan::Vulkan
            OpenGL::GL
    )
endif()

# TODO: Package project with CPACK