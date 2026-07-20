# Resolves to {package}/res relative to this file at {package}/cmake/
get_filename_component(AURORA_ENGINE_RES_DIR "${CMAKE_CURRENT_LIST_DIR}/../res" ABSOLUTE CACHE)

# Call this on a consumer target to have the engine's assets copied next to its executable.
#
# Usage:
#   aurora_engine_copy_assets(MyGame)
function(aurora_engine_copy_assets TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${AURORA_ENGINE_RES_DIR}"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/aurora-engine"
        COMMENT "Copying AuroraEngine assets to output..."
    )
endfunction()
