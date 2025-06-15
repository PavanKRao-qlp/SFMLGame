
file(GLOB_RECURSE SOURCES_SANDBOX "src/*.cpp", )
add_executable(PhysicsTestBed  ${SOURCES_SANDBOX})
target_link_libraries(PhysicsTestBed Engine2D)
# Register this executable for DLL copying
setup_target_dll_copy(PhysicsTestBed)