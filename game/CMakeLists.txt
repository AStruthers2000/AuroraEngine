add_executable(AuroraEngineTest
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/player.cpp
)

# Include game directory for player.h
target_include_directories(AuroraEngineTest
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link to AuroraEngine library
target_link_libraries(AuroraEngineTest
    PRIVATE
        AuroraEngine
)
