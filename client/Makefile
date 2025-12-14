# Client Makefile for File Sharing System

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
INCLUDES = -I include -I lib/file_ops/include

# Directories
SRC_DIR = src
LIB_DIR = lib
BIN_DIR = bin
OBJ_DIR = obj

# Source files
MAIN_SRC = $(SRC_DIR)/client.c
LIB_SRCS = $(LIB_DIR)/file_ops/src/file_upload.c

# All source files
SRCS = $(MAIN_SRC) $(LIB_SRCS)

# Object files (derived from source files)
MAIN_OBJ = $(OBJ_DIR)/client.o
LIB_OBJS = $(OBJ_DIR)/file_upload.o
OBJS = $(MAIN_OBJ) $(LIB_OBJS)

# Output executable
TARGET = $(BIN_DIR)/client

# Default target
all: $(TARGET)

# Create necessary directories
$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

# Build executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "Linking $@..."
	$(CC) $(CFLAGS) $(OBJS) -o $@
	@echo "Build complete: $@"

# Compile main client
$(OBJ_DIR)/client.o: $(SRC_DIR)/client.c | $(OBJ_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile file upload module
$(OBJ_DIR)/file_upload.o: $(LIB_DIR)/file_ops/src/file_upload.c | $(OBJ_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BIN_DIR) $(OBJ_DIR)
	@echo "Clean complete"

# Rebuild (clean + build)
rebuild: clean all

# Run client
run: $(TARGET)
	@echo "Running client..."
	@./$(TARGET)

# Install dependencies (if needed)
deps:
	@echo "Checking dependencies..."
	@which gcc > /dev/null || (echo "gcc not found. Install with: sudo apt-get install build-essential" && exit 1)
	@echo "All dependencies satisfied"

# Help
help:
	@echo "Available targets:"
	@echo "  make all      - Build client (default)"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make rebuild  - Clean and build"
	@echo "  make run      - Build and run client"
	@echo "  make deps     - Check dependencies"
	@echo "  make help     - Show this help message"

# Phony targets (not actual files)
.PHONY: all clean rebuild run deps help
