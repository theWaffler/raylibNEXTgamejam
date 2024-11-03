#**************************************************************************************************
#
#   raylib makefile for Desktop platforms, Web (Wasm), Raspberry Pi (DRM mode) and Android
#
#   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)
#
#   This software is provided "as-is", without any express or implied warranty. In no event
#   will the authors be held liable for any damages arising from the use of this software.
#
#   Permission is granted to anyone to use this software for any purpose, including commercial
#   applications, and to alter it and redistribute it freely, subject to the following restrictions:
#
#     1. The origin of this software must not be misrepresented; you must not claim that you
#     wrote the original software. If you use this software in a product, an acknowledgment
#     in the product documentation would be appreciated but is not required.
#
#     2. Altered source versions must be plainly marked as such, and must not be misrepresented
#     as being the original software.
#
#     3. This notice may not be removed or altered from any source distribution.
#
#**************************************************************************************************

.PHONY: all clean

# Define required environment variables
#------------------------------------------------------------------------------------------------
# Define target platform: PLATFORM_DESKTOP, PLATFORM_WEB, PLATFORM_DRM, PLATFORM_ANDROID
PLATFORM              ?= PLATFORM_DESKTOP

# Define project variables
PROJECT_NAME          ?= terminal_infiltrator
PROJECT_VERSION       ?= 1.0
PROJECT_BUILD_PATH    ?= .
PROJECT_SOURCE_FILES  ?= main.cpp Game.cpp Terminal.cpp Directory.cpp File.cpp PopupDialog.cpp BreachProtocol.cpp

# raylib library variables
RAYLIB_SRC_PATH       ?= C:/raylib/raylib/src
RAYLIB_INCLUDE_PATH   ?= $(RAYLIB_SRC_PATH)
RAYLIB_LIB_PATH       ?= $(RAYLIB_SRC_PATH)

# Library type used for raylib: STATIC (.a) or SHARED (.so/.dll)
RAYLIB_LIBTYPE        ?= STATIC

# Define compiler path on Windows
COMPILER_PATH         ?= C:\raylib\w64devkit\bin

# Build mode for project: DEBUG or RELEASE
BUILD_MODE            ?= RELEASE

# PLATFORM_WEB: Default properties
BUILD_WEB_ASYNCIFY    ?= FALSE
BUILD_WEB_SHELL       ?= minshell.html
BUILD_WEB_HEAP_SIZE   ?= 128MB
BUILD_WEB_STACK_SIZE  ?= 1MB
BUILD_WEB_ASYNCIFY_STACK_SIZE ?= 1048576
BUILD_WEB_RESOURCES   ?= TRUE
BUILD_WEB_RESOURCES_PATH  ?= resources

# Rest of platform detection code remains the same...
[Previous platform detection code remains unchanged]

# Define C++ compiler
#------------------------------------------------------------------------------------------------
CXX = g++

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),OSX)
        CXX = clang++
    endif
    ifeq ($(PLATFORM_OS),BSD)
        CXX = clang++
    endif
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    CXX = em++
endif

# Define compiler flags: CFLAGS and CXXFLAGS
#------------------------------------------------------------------------------------------------
CFLAGS = -Wall -Wno-missing-braces -Wno-unused-value -D_DEFAULT_SOURCE $(PROJECT_CUSTOM_FLAGS)
CXXFLAGS = $(CFLAGS) -std=c++17

ifeq ($(BUILD_MODE),DEBUG)
    CXXFLAGS += -g -D_DEBUG
else
    ifeq ($(PLATFORM),PLATFORM_WEB)
        ifeq ($(BUILD_WEB_ASYNCIFY),TRUE)
            CXXFLAGS += -O3
        else
            CXXFLAGS += -Os
        endif
    else
        ifeq ($(PLATFORM_OS),OSX)
            CXXFLAGS += -O2
        else
            CXXFLAGS += -s -O2
        endif
    endif
endif

# Rest of INCLUDE_PATHS, LDFLAGS, and LDLIBS remain the same...
[Previous paths and flags sections remain unchanged]

# Define all object files from source files
#------------------------------------------------------------------------------------------------
OBJS = $(patsubst %.cpp, %.o, $(PROJECT_SOURCE_FILES))

# Define processes to execute
#------------------------------------------------------------------------------------------------
# For Android platform we call a custom Makefile.Android
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    MAKEFILE_TARGET = -f Makefile.Android
    export PROJECT_NAME
    export PROJECT_SOURCE_FILES
else
    MAKEFILE_TARGET = $(PROJECT_NAME)
endif

# Default target entry
all:
	$(MAKE) $(MAKEFILE_TARGET)

# Project target defined by PROJECT_NAME
$(PROJECT_NAME): $(OBJS)
	$(CXX) -o $(PROJECT_BUILD_PATH)/$(PROJECT_NAME)$(EXT) $(OBJS) $(CXXFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -D$(PLATFORM)

# Compile source files
# NOTE: This pattern will compile every module defined on $(OBJS)
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM)

# Clean everything
clean:
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
		del *.o *.exe /s
    endif
    ifeq ($(PLATFORM_OS),LINUX)
		find . -type f -executable -delete
		rm -fv *.o
    endif
    ifeq ($(PLATFORM_OS),OSX)
		rm -f *.o external/*.o $(PROJECT_NAME)
    endif
endif
ifeq ($(PLATFORM),PLATFORM_DRM)
	find . -type f -executable -delete
	rm -fv *.o
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
	del *.o *.html *.js
endif
	@echo Cleaning done
