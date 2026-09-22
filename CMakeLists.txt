cmake_minimum_required(VERSION 3.20)

project(Hexel
    VERSION 1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DTEST_INPUT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test-input/\")
add_definitions(-DSHADERS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/\")


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
# for windows user (nico): Put all dependencies in a folder of the .sln
# ============================================================
function(set_folder_recursive dir folder)
    get_property(targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS targets)
        set_target_properties("${target}"
            PROPERTIES FOLDER "${folder}"
        )
    endforeach()
    get_property(subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
    foreach(subdir IN LISTS subdirs)
        set_folder_recursive("${subdir}" "${folder}")
    endforeach()
endfunction()

set_folder_recursive(
    "${fmt_SOURCE_DIR}"
    "Dependencies/fmt"
)

# ============================================================
# Application
# ============================================================

add_executable(Hexel
    src/main.cpp
    src/core/basic.h
    src/core/picker.h
    src/core/panels.h
    src/core/layers.h
    src/core/shaders.h
    src/core/core.h src/core/core.cpp
    src/core/window.h src/core/window.cpp
    src/core/camera.h src/core/camera.cpp
    src/core/xcf.h src/core/xcf.cpp
)

target_link_libraries(Hexel PRIVATE
    ultimaille
    imgui
    glad_gl_core_33
    OpenGL::GL
)

