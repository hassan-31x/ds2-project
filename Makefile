.PHONY: all clean raylib

# Detect the operating system
ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell uname)
endif

# Project configuration
PROJECT_NAME   = class_scheduler
CC             = clang++
SRC_DIR        = src
OBJ_DIR        = obj
BIN_DIR        = .
BINARY         = $(BIN_DIR)/$(PROJECT_NAME)
CFLAGS         = -Wall -std=c++11 -D_DEFAULT_SOURCE -Wno-missing-braces -O2

# Modify the include paths to use raylib locally
INCLUDE_PATHS  = -I. -I$(SRC_DIR) -I./raylib/include

# Remove raylib dependencies for testing - just use header files
LDFLAGS        = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

SOURCES        = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS        = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Default build target
all: create_dirs $(BINARY)

# Create the necessary directories
create_dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# Build the executable
$(BINARY): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)
	@echo "Build complete! Run with ./$(PROJECT_NAME)"

# Compile individual source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_PATHS)

# Clean the build directory
clean:
	rm -rf $(OBJ_DIR)
	rm -f $(BINARY)

# Run the project
run: all
	./$(PROJECT_NAME)

# Build raylib locally if it doesn't exist
raylib:
	@echo "Building raylib..."
	@if [ ! -d raylib/lib ]; then mkdir -p raylib/lib; fi
	@gcc -c ./raylib/include/raylib.h -o ./raylib/lib/raylib.o -I./raylib/include
	@ar rcs ./raylib/lib/libraylib.a ./raylib/lib/raylib.o
	@echo "Raylib built successfully."