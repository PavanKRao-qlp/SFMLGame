
file(GLOB_RECURSE SOURCES_SANDBOX "src/*.cpp", )
add_executable(PhysicsTestBed  ${SOURCES_SANDBOX})
target_link_libraries(PhysicsTestBed Engine2D)
setup_target_dll_copy(PhysicsTestBed)
setup_target_asset_copy(PhysicsTestBed)