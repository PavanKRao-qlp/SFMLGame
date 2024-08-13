cmake_minimum_required(VERSION 3.5.0)
project(SFML2D VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include SFML headers
include_directories(Engine/include/)

# Add all source files from Game/src
file(GLOB_RECURSE SOURCES_ENGINE "Engine/src/*.cpp")

add_library(Engine2D STATIC ${SOURCES_ENGINE})
target_precompile_headers(Engine2D PUBLIC "Engine/include/EnginePCH.h")


file(GLOB_RECURSE SOURCES_SANDBOX "Sandbox/SimpleSandbox/src/*.cpp")
add_executable(SimpleSandbox  ${SOURCES_SANDBOX})
target_link_libraries(SimpleSandbox Engine2D)
#target_link_libraries(Engine2D)
# Add the executable
#add_executable(SFML2D ${SOURCES_ENGINE})
# Link SFML libraries
#target_link_libraries(Engine2d sfml-graphics sfml-window sfml-system)