cmake_minimum_required(VERSION 3.16)
project(games101_renderer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ★ 强制 Release：Debug 的性能数据没有意义
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

if(MSVC)
    add_compile_options(/utf-8 /MP /EHsc /W3)
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-sign-compare)
endif()

# 替代 A5 里硬编码的 -fsanitize=undefined
option(G101_UBSAN "Enable undefined-behavior sanitizer" OFF)
if(G101_UBSAN AND NOT MSVC)
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined)
endif()

option(G101_OPENMP "Enable OpenMP for path tracing" ON)
if(G101_OPENMP)
    find_package(OpenMP QUIET)
    if(OpenMP_CXX_FOUND)
        set(G101_HAS_OPENMP TRUE)
    endif()
endif()



add_library(g101_core STATIC src/core/image.cpp)
target_include_directories(g101_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/eigen
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/obj_loader
)
if(G101_BUILD_VIEWER AND UNIX AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/viewer)
    add_subdirectory(viewer)
endif()
if(G101_HAS_OPENMP)
    target_link_libraries(g101_core PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(g101_core PUBLIC G101_HAS_OPENMP=1)
endif()

add_library(g101_raster   STATIC src/raster/rasterizer.cpp src/raster/texture.cpp src/raster/triangle.cpp)
add_library(g101_geometry STATIC src/geometry/bezier.cpp)
add_library(g101_raytrace STATIC src/raytrace/bvh.cpp src/raytrace/scene.cpp src/raytrace/integrator.cpp)
add_library(g101_physics  STATIC src/physics/rope_sim.cpp)
foreach(m g101_raster g101_geometry g101_raytrace g101_physics)
    target_link_libraries(${m} PUBLIC g101_core)
endforeach()


# ② renderer 拆成 6 个源文件（解决 Vector3f 二义）
add_executable(renderer
    apps/main.cpp
    apps/raytrace_app.cpp
    apps/raster_app.cpp
    apps/geometry_app.cpp
    apps/rope_app.cpp
    apps/scene_presets.cpp
)
target_link_libraries(renderer PRIVATE g101_raster g101_geometry g101_raytrace g101_physics)
target_compile_definitions(renderer PRIVATE
    G101_DEFAULT_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")

# 可选：原 CGL 交互查看器（只支持 OpenGL 的平台）
option(G101_BUILD_VIEWER "Build legacy CGL rope viewer" OFF)
if(G101_BUILD_VIEWER AND UNIX)
    add_subdirectory(viewer)
endif()