cmake_minimum_required(VERSION 3.0)
project(terminal_infiltrator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add raylib
find_package(raylib REQUIRED)

# Source files - now relative to src directory
set(SOURCES
    main.cpp
    Game.cpp
    Terminal.cpp
    Directory.cpp
    File.cpp
    PopupDialog.cpp
    BreachProtocol.cpp
)

# Create executable
add_executable(terminal_infiltrator ${SOURCES})

# Include directories (current directory for headers)
target_include_directories(terminal_infiltrator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries
target_link_libraries(terminal_infiltrator PRIVATE raylib)
if(NOT WIN32)
    target_link_libraries(terminal_infiltrator PRIVATE m)
endif()

# Web Configurations
if (${PLATFORM} STREQUAL "Web")
    # Set HTML output
    set_target_properties(terminal_infiltrator PROPERTIES SUFFIX ".html")
    
    # Configure Emscripten flags
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s FORCE_FILESYSTEM=1 -s WASM=1")
    
    # Debug configurations
    set(web_link_flags "")
    if(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
        set(web_link_flags "${web_link_flags} -s ASSERTIONS=1")
    endif()
    
    # Resource handling - now relative to src directory
    set(web_link_flags "${web_link_flags} --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/resources@resources --use-preload-plugins")
    set(web_link_flags "${web_link_flags} --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/minshell.html")
    
    # Apply web flags
    set_target_properties(terminal_infiltrator PROPERTIES LINK_FLAGS "${web_link_flags}")
    
    # Additional Emscripten-specific flags for C++
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1 -s DISABLE_EXCEPTION_CATCHING=0")
endif()

# MacOS specific configurations
if(APPLE)
    target_link_libraries(terminal_infiltrator PRIVATE
        "-framework IOKit"
        "-framework Cocoa"
        "-framework OpenGL"
    )
endif()

# Set working directory for Visual Studio
set_target_properties(terminal_infiltrator PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# Set as startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
    PROPERTY VS_STARTUP_PROJECT terminal_infiltrator)
