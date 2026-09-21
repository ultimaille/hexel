cmake_minimum_required(VERSION 3.20)

project(Hexel
    VERSION 1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ============================================================
# GLFW
# ============================================================

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw)


# ============================================================
# GLAD 2
# ============================================================

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR cmake
)

FetchContent_MakeAvailable(glad)

glad_add_library(
    glad_gl_core_33
    STATIC
    REPRODUCIBLE
    LOADER
    API gl:core=3.3
)


# ============================================================
# Dear ImGui
# ============================================================

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.2
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(imgui)


add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp

    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_compile_definitions(imgui PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_GLAD2
)

target_link_libraries(imgui PUBLIC
    glfw
    glad_gl_core_33
)


# ============================================================
# Ultimaille
# ============================================================

FetchContent_Declare(
    ultimaille
    GIT_REPOSITORY https://github.com/ultimaille/ultimaille.git
    GIT_TAG master
)

FetchContent_MakeAvailable(ultimaille)
include_directories(${ultimaille_SOURCE_DIR})

# ============================================================
# OpenGL
# ============================================================

find_package(OpenGL REQUIRED)


# ============================================================
# Application
# ============================================================

add_executable(Hexel
    src/main.cpp
)


target_link_libraries(Hexel PRIVATE
    ultimaille
    imgui
    glad_gl_core_33
    OpenGL::GL
)