cmake_minimum_required(VERSION 3.28)
project(AuroraEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to build test executable (disabled during Conan packaging)
option(BUILD_TESTING "Build test executable" ON)

# Find external dependencies
find_package(glm CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

# Library
add_library(AuroraEngine
    ${PROJECT_SOURCE_DIR}/src/engine.cpp
    ${PROJECT_SOURCE_DIR}/src/game_object.cpp
    ${PROJECT_SOURCE_DIR}/src/window.cpp
    ${PROJECT_SOURCE_DIR}/src/world.cpp

    ${PROJECT_SOURCE_DIR}/src/subsystems/input_subsystem.cpp
)

# Include directories
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

# Test executable (only built during local development)
if(BUILD_TESTING)
    add_executable(AuroraEngineTest
        ${PROJECT_SOURCE_DIR}/game/main.cpp
        ${PROJECT_SOURCE_DIR}/game/player.cpp
    )

    # Include game directory for player.h
    target_include_directories(AuroraEngineTest
        PRIVATE
            ${PROJECT_SOURCE_DIR}/game
    )

    # Link to AuroraEngine library
    target_link_libraries(AuroraEngineTest
        PRIVATE
            AuroraEngine
    )
endif()
