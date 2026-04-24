cmake_minimum_required(VERSION 3.15)
project(Hello)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)


find_package(Qt6 REQUIRED COMPONENTS Widgets)
if (NOT Qt6_FOUND)
message(FATAL_ERROR "QT6 Required for building the project")
endif()

add_subdirectory(external/glm)

add_executable(${PROJECT_NAME}  kernel/main.cpp
                                kernel/rasterization/algorithm.cpp
                                kernel/rasterization/rasterization.cpp
                                kernel/geometry/geometry.cpp
                                kernel/screen/screen.cpp
                                kernel/world/object.cpp
                                kernel/world/world.cpp
                                kernel/camera/camera.cpp
                                kernel/renderer/renderer.cpp
                                kernel/parser/parser.cpp
                                kernel/application/application.cpp
                                kernel/concurrency/concurrency.cpp
                                kernel/textures/textures.cpp
                                kernel/exceptions/exceptions.cpp
                                kernel/light/light.cpp
                                kernel/shadows/shadow_map.cpp
                                )

target_include_directories(${PROJECT_NAME} PRIVATE kernel)

target_link_libraries(${PROJECT_NAME} Qt6::Widgets)
target_link_libraries(${PROJECT_NAME} glm::glm)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -O3
        -march=native
        -ffast-math
        -funroll-loops
    )
endif()
