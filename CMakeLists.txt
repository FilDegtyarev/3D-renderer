cmake_minimum_required(VERSION 3.15)
project(Hello)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

find_package(Qt6 REQUIRED)
find_package(glm REQUIRED)

add_executable(${PROJECT_NAME}  kernel/main.cpp
                                kernel/rasterization/algorithm.cpp
                                kernel/rasterization/rasterization.cpp
                                kernel/geometry/geometry.cpp
                                kernel/screen/screen.cpp
                                kernel/world/object.cpp
                                kernel/world/world.cpp
                                kernel/camera/camera.cpp
                                kernel/renderer/renderer.cpp
                                )


target_include_directories(${PROJECT_NAME} PRIVATE kernel)

target_link_libraries(${PROJECT_NAME} Qt6::Widgets)
target_link_libraries(${PROJECT_NAME} glm::glm)