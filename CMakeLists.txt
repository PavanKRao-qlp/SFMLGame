cmake_minimum_required(VERSION 3.5.0)
project(SFML2D VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add SFML as a submodule
add_subdirectory(external/SFML) 
#add_subdirectory(external/PakFileSystem) 

# Include SFML headers
include_directories(Engine/include/ external/SFML/include)
# external/PakFileSystem/PakFileSystem/include)

# Add all source files from Game/src
file(GLOB_RECURSE SOURCES "Engine/src/*.cpp" "Engine/src/*.h" )

# Add the executable
add_executable(SFML2D ${SOURCES})

# Link SFML libraries
target_link_libraries(SFML2D sfml-graphics sfml-window sfml-system)