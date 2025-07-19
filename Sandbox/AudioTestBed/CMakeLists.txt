
file(GLOB_RECURSE SOURCES_SANDBOX "src/*.cpp", )
add_executable(AudioTestBed  ${SOURCES_SANDBOX})
target_link_libraries(AudioTestBed Engine2D)
# Register this executable for DLL copying
setup_target_dll_copy(AudioTestBed)

add_custom_command(
    TARGET AudioTestBed POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Asset"
            "$<TARGET_FILE_DIR:AudioTestBed>/Asset"
    COMMENT "Copying assets folder to the build directory"
)