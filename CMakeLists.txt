cmake_minimum_required(VERSION 3.5.0)
project(SFML2D VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "Use find_package to find SFML" FORCE)
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui" CACHE PATH "Path to ImGui")
# Define a global list to collect all executable targets
# This needs to be defined BEFORE adding subdirectories
set(ALL_EXE_TARGETS "" CACHE INTERNAL "List of all executable targets")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)
add_subdirectory(external/SFML)
add_subdirectory(external/glm)
add_subdirectory(external/imgui-sfml)
add_subdirectory(external/miniaudio)
add_subdirectory(external/LDtkLoader)
#add_subdirectory(external/hlslpp)
include_directories(
    Engine/include/
    external/SFML/include
    external/glm
    external/imgui
    external/imgui-sfml
    external/miniaudio
    external/LDtkLoader/include
 )

# Add all source files from Game/src
file(GLOB_RECURSE SOURCES_ENGINE "Engine/src/*.cpp")

add_library(Engine2D STATIC ${SOURCES_ENGINE} ${IMGUI_SFML_SOURCES})
target_link_libraries(Engine2D
sfml-graphics
sfml-window
sfml-system
ImGui-SFML
miniaudio
LDtkLoader
)
target_precompile_headers(Engine2D PUBLIC "Engine/include/EnginePCH.h")

# Define a function to register an executable for DLL copying later
# This function should be called from subdirectories
function(register_executable_for_dll_copy TARGET_NAME)
    # Add the target to our global list
  #  set(ALL_EXE_TARGETS ${ALL_EXE_TARGETS} ${TARGET_NAME} CACHE INTERNAL "")
    message(STATUS "Registered ${TARGET_NAME} for DLL copying")
endfunction()

macro(setup_target_dll_copy TARGET_NAME)
    # Only do this on Windows and if copying is enabled
    if(WIN32 )
        # Get paths to all SFML DLLs using generator expressions
        set(REQUIRED_DLLS
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE:ImGui-SFML>
            # Add other DLLs as needed
        )
        
        # Add the post-build command to copy DLLs
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying DLLs for ${TARGET_NAME}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${REQUIRED_DLLS}
            $<TARGET_FILE_DIR:${TARGET_NAME}>
            COMMENT "Copying DLLs to ${TARGET_NAME} output directory"
        )
        
        # Optional: Print a status message for debugging
        message(STATUS "DLL copying set up for target: ${TARGET_NAME}")
    endif()
endmacro()

add_subdirectory(Project/SimpleSandbox)
add_subdirectory(Project/PhysicsTestBed)
add_subdirectory(Project/PhysicsEngineDemo)
add_subdirectory(Project/AudioTestBed)
add_subdirectory(Project/MultiThreading)
#add_subdirectory(Project/JobSystem)

