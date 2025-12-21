# Unified Makefile for File Sharing System
# Builds both client and server from root directory

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

# Directories
SERVER_DIR = server
CLIENT_DIR = client
BIN_DIR = bin

# --- SERVER CONFIGURATION ---
SERVER_INCLUDES = -I $(SERVER_DIR)/include \
                  -I $(SERVER_DIR)/lib/auth/include \
                  -I $(SERVER_DIR)/lib/session/include \
                  -I $(SERVER_DIR)/lib/protocol/include \
                  -I $(SERVER_DIR)/lib/file_ops/include \
				  -I $(SERVER_DIR)/lib/group/include

SERVER_SRCS = $(SERVER_DIR)/src/server.c \
              $(SERVER_DIR)/lib/auth/src/user.c \
              $(SERVER_DIR)/lib/auth/src/register.c \
              $(SERVER_DIR)/lib/auth/src/login.c \
              $(SERVER_DIR)/lib/session/src/session.c \
              $(SERVER_DIR)/lib/protocol/src/parser.c \
              $(SERVER_DIR)/lib/protocol/src/handlers.c \
              $(SERVER_DIR)/lib/file_ops/src/file_transfer.c \
			  $(SERVER_DIR)/lib/group/src/group_read.c \
			  $(SERVER_DIR)/lib/group/src/group_write.c \
			  $(SERVER_DIR)/lib/group/src/group_repo.c \
			  $(SERVER_DIR)/lib/group/src/group.c \

SERVER_TARGET = $(BIN_DIR)/server

# --- CLIENT CONFIGURATION ---
CLIENT_INCLUDES = -I $(CLIENT_DIR)/include \
                  -I $(CLIENT_DIR)/lib/file_ops/include

CLIENT_SRCS = $(CLIENT_DIR)/src/client.c \
              $(CLIENT_DIR)/lib/file_ops/src/file_upload.c

CLIENT_TARGET = $(BIN_DIR)/client

# --- TARGETS ---

.PHONY: all client server clean setup help

# Default: build both
all: directories server client
	@echo ""
	@echo "✓ Build complete!"
	@echo "  Server: $(SERVER_TARGET)"
	@echo "  Client: $(CLIENT_TARGET)"

# Build server
server: directories $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_SRCS)
	@echo "Building server..."
	$(CC) $(CFLAGS) $(SERVER_INCLUDES) -pthread $(SERVER_SRCS) $(GROUP_TEST) -o $@
	@echo "✓ Server built: $@"

# Build client
client: directories $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_SRCS)
	@echo "Building client..."
	$(CC) $(CFLAGS) $(CLIENT_INCLUDES) $(CLIENT_SRCS) -o $@
	@echo "✓ Client built: $@"

# Create directories
directories:
	@mkdir -p $(BIN_DIR)

# Setup runtime directories
setup: directories
	@echo "Creating runtime directories..."
	@mkdir -p $(SERVER_DIR)/database
	@mkdir -p $(SERVER_DIR)/storage
	@echo "✓ Setup complete (database/ and storage/)"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BIN_DIR)
	@rm -rf $(SERVER_DIR)/bin $(SERVER_DIR)/obj
	@rm -rf $(CLIENT_DIR)/bin $(CLIENT_DIR)/obj
	@echo "✓ Clean complete"

# Clean everything including runtime data
clean-all: clean
	@echo "Cleaning runtime data..."
	@rm -rf $(SERVER_DIR)/database/*
	@rm -rf $(SERVER_DIR)/storage/*
	@echo "✓ All data cleaned"

# Rebuild
rebuild: clean all

# Run server
run-server: server setup
	@echo "Starting server..."
	@cd $(SERVER_DIR) && ../$(SERVER_TARGET)

# Run client
run-client: client
	@echo "Starting client..."
	@./$(CLIENT_TARGET)

# Help
help:
	@echo "File Sharing System - Unified Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  make all         - Build both client and server (default)"
	@echo "  make client      - Build only client"
	@echo "  make server      - Build only server"
	@echo "  make clean       - Remove build artifacts"
	@echo "  make clean-all   - Remove build artifacts and runtime data"
	@echo "  make rebuild     - Clean and rebuild"
	@echo "  make setup       - Create runtime directories"
	@echo "  make run-server  - Build and run server"
	@echo "  make run-client  - Build and run client"
	@echo "  make help        - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build everything"
	@echo "  make server && make run-server"
	@echo "  make client && make run-client"

# --- TEST CONFIGURATION ---
TEST_DIR = $(SERVER_DIR)/test
TEST_INCLUDES = $(SERVER_INCLUDES) -I $(SERVER_DIR)/lib/utils

# Protocol parser test
PROTOCOL_TEST_SRCS = $(SERVER_DIR)/lib/protocol/test/protocol_parser_test.c \
                     $(SERVER_DIR)/lib/protocol/src/parser.c

PROTOCOL_TEST_TARGET = $(BIN_DIR)/test_protocol_parser

# --- TEST TARGETS ---

.PHONY: test

# Build and run protocol parser tests
test: directories $(PROTOCOL_TEST_TARGET)
	@echo "Running protocol parser tests..."
	@./$(PROTOCOL_TEST_TARGET)

$(PROTOCOL_TEST_TARGET): $(PROTOCOL_TEST_SRCS)
	@echo "Building protocol parser tests..."
	$(CC) $(CFLAGS) $(TEST_INCLUDES) $(PROTOCOL_TEST_SRCS) -o $@
