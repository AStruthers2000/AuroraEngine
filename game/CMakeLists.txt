add_executable(AuroraEngineTest
    ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
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

# add_custom_command(TARGET AuroraEngineTest POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#         "${CMAKE_CURRENT_SOURCE_DIR}/data"
#         "$<TARGET_FILE_DIR:AuroraEngineTest>/data"
#     COMMENT "Copying game assets to output directory"
# )
