
file(GLOB_RECURSE SOURCES_SIMPLESANDBOX "src/*.cpp", )
add_executable(SimpleSandbox  ${SOURCES_SIMPLESANDBOX})
target_link_libraries(SimpleSandbox Engine2D)

# Register this executable for DLL copying
setup_target_dll_copy(SimpleSandbox)

add_custom_command(
    TARGET SimpleSandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Asset"
            "$<TARGET_FILE_DIR:SimpleSandbox>/Asset"
    COMMENT "Copying assets folder to the build directory"
)