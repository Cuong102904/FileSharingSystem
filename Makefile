# Unified Makefile for File Sharing System (server + client)
# Rebuilt to include all source files across modules

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -pthread

# Directories
SERVER_DIR = server
CLIENT_DIR = client
BIN_DIR = bin

# Include paths
SERVER_INCLUDES = \
	-I $(SERVER_DIR)/include \
	-I $(SERVER_DIR)/lib/auth/include \
	-I $(SERVER_DIR)/lib/session/include \
	-I $(SERVER_DIR)/lib/protocol/include \
	-I $(SERVER_DIR)/lib/file_ops/include \
	-I $(SERVER_DIR)/lib/group/include \
	-I $(SERVER_DIR)/lib/utils

CLIENT_INCLUDES = \
	-I $(CLIENT_DIR)/include \
	-I $(CLIENT_DIR)/lib/file_ops/include

# --- SERVER SOURCES ---
SERVER_SRCS = \
	$(SERVER_DIR)/src/server.c \
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
	$(SERVER_DIR)/lib/group/src/group.c

SERVER_TARGET = $(BIN_DIR)/server

# --- CLIENT SOURCES ---
CLIENT_SRCS = \
	$(CLIENT_DIR)/src/client.c \
	$(CLIENT_DIR)/lib/file_ops/src/file_upload.c

CLIENT_TARGET = $(BIN_DIR)/client

# --- TESTS ---
TEST_INCLUDES = $(SERVER_INCLUDES)
PROTOCOL_TEST_SRCS = \
	$(SERVER_DIR)/lib/protocol/test/protocol_parser_test.c \
	$(SERVER_DIR)/lib/protocol/src/parser.c
PROTOCOL_TEST_TARGET = $(BIN_DIR)/test_protocol_parser

# Phony
.PHONY: all server client test clean clean-all setup run-server run-client rebuild directories

# Default: build both
all: directories server client
	@echo ""
	@echo "✓ Build complete!"
	@echo "  Server: $(SERVER_TARGET)"
	@echo "  Client: $(CLIENT_TARGET)"

# Create bin directory
directories:
	@mkdir -p $(BIN_DIR)

# Build server
server: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_SRCS)
	@echo "Building server..."
	$(CC) $(CFLAGS) $(SERVER_INCLUDES) $(LDFLAGS) $(SERVER_SRCS) -o $@
	@echo "✓ Server built: $@"

# Build client
client: $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_SRCS)
	@echo "Building client..."
	$(CC) $(CFLAGS) $(CLIENT_INCLUDES) $(CLIENT_SRCS) -o $@
	@echo "✓ Client built: $@"

# Build and run protocol parser tests
test: directories $(PROTOCOL_TEST_TARGET)
	@echo "Running protocol parser tests..."
	@./$(PROTOCOL_TEST_TARGET)

$(PROTOCOL_TEST_TARGET): $(PROTOCOL_TEST_SRCS)
	@echo "Building protocol parser tests..."
	$(CC) $(CFLAGS) $(TEST_INCLUDES) $(PROTOCOL_TEST_SRCS) -o $@

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
