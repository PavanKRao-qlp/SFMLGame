file(GLOB_RECURSE SOURCES_AUDIO_TESTBED "src/*.cpp")
add_executable(AudioTestBed ${SOURCES_AUDIO_TESTBED})
target_link_libraries(AudioTestBed Engine2D)
setup_target_dll_copy(AudioTestBed)

# Copy audio assets to output directory
add_custom_command(TARGET AudioTestBed POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/Asset"
    "$<TARGET_FILE_DIR:AudioTestBed>/Asset"
    COMMENT "Copying Asset folder to AudioTestBed output directory"
)
