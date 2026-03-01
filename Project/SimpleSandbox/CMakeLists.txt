
file(GLOB_RECURSE SOURCES_SIMPLESANDBOX "src/*.cpp", )
add_executable(SimpleSandbox  ${SOURCES_SIMPLESANDBOX})
target_link_libraries(SimpleSandbox Engine2D)

setup_target_dll_copy(SimpleSandbox)
setup_target_asset_copy(SimpleSandbox)