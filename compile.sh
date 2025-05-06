#!/bin/bash

# Create obj directory if it doesn't exist
mkdir -p obj

# Clean up previous build
rm -f scheduler
rm -f obj/*.o

# Detect OS for proper library paths
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    echo "Building for macOS..."
    
    # Use local raylib includes
    RAYLIB_INCLUDE_PATH="./raylib/include"
    
    # Find the raylib library
    if [ -f "./libraylib.a" ]; then
        echo "Using raylib from root directory"
        RAYLIB_LIB_PATH="."
    elif [ -d "/opt/homebrew/Cellar/raylib" ]; then
        # Try Homebrew path on Apple Silicon Macs
        RAYLIB_LIB_PATH=$(find /opt/homebrew/Cellar/raylib -name "lib" -type d | head -n 1)
        echo "Using raylib from Homebrew: $RAYLIB_LIB_PATH"
    elif [ -d "/usr/local/Cellar/raylib" ]; then
        # Try Homebrew path on Intel Macs
        RAYLIB_LIB_PATH=$(find /usr/local/Cellar/raylib -name "lib" -type d | head -n 1)
        echo "Using raylib from Homebrew: $RAYLIB_LIB_PATH"
    elif [ -f "/usr/local/lib/libraylib.a" ]; then
        RAYLIB_LIB_PATH="/usr/local/lib"
        echo "Using raylib from /usr/local/lib"
    else
        echo "ERROR: Could not find a valid libraylib.a"
        echo "Please run 'brew install raylib' or build raylib from source"
        exit 1
    fi
    
    # Compile all source files
    for file in src/*.cpp; do
        echo "Compiling $file..."
        g++ -c "$file" -o "obj/$(basename "${file%.*}").o" -std=c++17 -Wall -Wextra -I./ -I"$RAYLIB_INCLUDE_PATH"
        
        # Check for compilation errors
        if [ $? -ne 0 ]; then
            echo "ERROR: Failed to compile $file"
            exit 1
        fi
    done
    
    # Link object files with raylib
    echo "Linking with raylib from: $RAYLIB_LIB_PATH"
    g++ obj/*.o -o scheduler -L"$RAYLIB_LIB_PATH" -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
    # Linux/Other
    echo "Building for Linux..."
    
    # Use local raylib includes
    RAYLIB_INCLUDE_PATH="./raylib/include"
    
    # Find the raylib library
    if [ -f "./libraylib.a" ]; then
        echo "Using raylib from root directory"
        RAYLIB_LIB_PATH="."
    elif [ -f "/usr/local/lib/libraylib.a" ]; then
        RAYLIB_LIB_PATH="/usr/local/lib"
        echo "Using raylib from /usr/local/lib"
    else
        echo "ERROR: Could not find a valid libraylib.a"
        echo "Please install raylib using your package manager or build from source"
        exit 1
    fi
    
    # Compile all source files
    for file in src/*.cpp; do
        echo "Compiling $file..."
        g++ -c "$file" -o "obj/$(basename "${file%.*}").o" -std=c++17 -Wall -Wextra -I./ -I"$RAYLIB_INCLUDE_PATH"
        
        # Check for compilation errors
        if [ $? -ne 0 ]; then
            echo "ERROR: Failed to compile $file"
            exit 1
        fi
    done
    
    # Link object files with raylib
    echo "Linking with raylib from: $RAYLIB_LIB_PATH"
    g++ obj/*.o -o scheduler -L"$RAYLIB_LIB_PATH" -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
fi

# Check if build was successful
if [ -f "./scheduler" ]; then
    # Make the executable executable
    chmod +x scheduler
    echo "Build complete! Run with ./scheduler"
else
    echo "Build failed!"
fi 