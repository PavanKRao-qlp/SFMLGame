cmake_minimum_required(VERSION 3.5.0)
project(SFML2D VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/SFML) 
include_directories(Engine/include/ external/SFML/include)

# Add all source files from Game/src
file(GLOB_RECURSE SOURCES_ENGINE "Engine/src/*.cpp")

add_library(Engine2D STATIC ${SOURCES_ENGINE})
target_link_libraries(Engine2D sfml-graphics sfml-window sfml-system)
target_precompile_headers(Engine2D PUBLIC "Engine/include/EnginePCH.h")


file(GLOB_RECURSE SOURCES_SANDBOX "Sandbox/SimpleSandbox/src/*.cpp")
add_executable(SimpleSandbox  ${SOURCES_SANDBOX})
target_link_libraries(SimpleSandbox Engine2D)