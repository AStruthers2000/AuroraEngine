cmake_minimum_required(VERSION 3.28)
project(AuroraEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to build test game (disabled during Conan packaging)
option(BUILD_TESTING "Build test game executable" ON)

# Find external dependencies
find_package(glm CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

# Engine library
add_library(AuroraEngine
    # Core
    ${PROJECT_SOURCE_DIR}/src/component.cpp
    ${PROJECT_SOURCE_DIR}/src/engine.cpp
    ${PROJECT_SOURCE_DIR}/src/entity.cpp
    ${PROJECT_SOURCE_DIR}/src/layer.cpp
    ${PROJECT_SOURCE_DIR}/src/window.cpp

    # Components
    # ${PROJECT_SOURCE_DIR}/src/components/collision_component.cpp
    ${PROJECT_SOURCE_DIR}/src/components/render_component.cpp
    ${PROJECT_SOURCE_DIR}/src/components/render_components/rect_render_component.cpp

    # Events
    ${PROJECT_SOURCE_DIR}/public/core/events/event.h
    ${PROJECT_SOURCE_DIR}/public/core/events/event_dispatcher.h
    ${PROJECT_SOURCE_DIR}/public/core/events/key_input_events.h

    # Subsystems
    # ${PROJECT_SOURCE_DIR}/src/subsystems/collision_subsystem.cpp
)

# Include directories for the engine
target_include_directories(AuroraEngine
    PUBLIC
        ${PROJECT_SOURCE_DIR}/public
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

# Link dependencies to engine
target_link_libraries(AuroraEngine
    PUBLIC
        glm::glm
        SDL3::SDL3
        SDL3_image::SDL3_image
)

# Test game executable (only built during local development)
if(BUILD_TESTING)
    add_subdirectory(game)
endif()
