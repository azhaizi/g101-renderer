cmake_minimum_required(VERSION 3.16)
project(Rasterizer)

find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(/usr/include)

add_executable(Rasterizer main.cpp rasterizer.hpp rasterizer.cpp Triangle.hpp Triangle.cpp)

target_include_directories(
    Rasterizer PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    /usr/include/eigen3
)
target_link_libraries(Rasterizer PRIVATE ${OpenCV_LIBS})
