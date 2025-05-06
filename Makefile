.PHONY: all clean

# Project configuration
PROJECT_NAME        = class_scheduler
RAYLIB_VERSION      = 4.2.0
PLATFORM           ?= PLATFORM_DESKTOP
BUILD_MODE         ?= RELEASE

# Compiler and flags
CC                 ?= g++
CFLAGS              = -Wall -std=c++11 -D_DEFAULT_SOURCE -Wno-missing-braces

# Detect OS for proper paths
ifeq ($(shell uname), Darwin)
    # macOS specific settings
    PLATFORM_OS     = OSX
    RAYLIB_PATH     = /usr/local/opt/raylib
    INCLUDE_PATHS   = -I. -Isrc -I$(RAYLIB_PATH)/include
    LDFLAGS         = -L$(RAYLIB_PATH)/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    CC              = clang++
else ifeq ($(OS),Windows_NT)
    # Windows specific settings
    PLATFORM_OS     = WINDOWS
    RAYLIB_PATH    ?= $(CURDIR)/raylib
    INCLUDE_PATHS   = -I. -Isrc -I$(RAYLIB_PATH)/include
    LDFLAGS         = -L$(RAYLIB_PATH)/lib -lraylib -lopengl32 -lgdi32 -lwinmm
else
    # Linux specific settings
    PLATFORM_OS     = LINUX
    RAYLIB_PATH    ?= /usr/local
    INCLUDE_PATHS   = -I. -Isrc -I$(RAYLIB_PATH)/include
    LDFLAGS         = -L$(RAYLIB_PATH)/lib -lraylib -lGL -lpthread -ldl -lrt -lX11
endif

# Debug vs Release settings
ifeq ($(BUILD_MODE), DEBUG)
    CFLAGS += -g -O0
else
    CFLAGS += -O2
endif

# Source files and objects
SRC_DIR = src
OBJ_DIR = obj
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

# Main targets
all: $(PROJECT_NAME)

$(PROJECT_NAME): create_dirs $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)
	@echo "Build complete! Run with ./$(PROJECT_NAME)"

# Create build directories
create_dirs:
	@mkdir -p $(OBJ_DIR)

# Compile rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_PATHS)

# Clean rule
clean:
	rm -rf $(OBJ_DIR)
	rm -f $(PROJECT_NAME)
	@echo "Cleanup complete!"

# Run the app
run: all
	./$(PROJECT_NAME)