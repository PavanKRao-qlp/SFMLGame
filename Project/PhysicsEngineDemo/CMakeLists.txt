file(GLOB_RECURSE SOURCES_PHYSICS_ENGINE_DEMO "src/*.cpp")
add_executable(PhysicsEngineDemo ${SOURCES_PHYSICS_ENGINE_DEMO})
target_link_libraries(PhysicsEngineDemo Engine2D)
setup_target_dll_copy(PhysicsEngineDemo)
