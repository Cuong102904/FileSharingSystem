# Makefile for Client-Server Network Programming Project

CC = gcc
CFLAGS = -Wall -pthread -I server/include

# Directories
SERVER_DIR = server
CLIENT_DIR = client
BIN_DIR = bin
OBJ_DIR = obj

# Server source files
SERVER_SRC = $(SERVER_DIR)/src/server.c
AUTH_SRC = $(SERVER_DIR)/lib/auth/src/user.c \
           $(SERVER_DIR)/lib/auth/src/register.c \
           $(SERVER_DIR)/lib/auth/src/login.c
SESSION_SRC = $(SERVER_DIR)/lib/session/src/session.c
PROTOCOL_SRC = $(SERVER_DIR)/lib/protocol/src/parser.c \
               $(SERVER_DIR)/lib/protocol/src/handlers.c

# All server sources
ALL_SERVER_SRC = $(SERVER_SRC) $(AUTH_SRC) $(SESSION_SRC) $(PROTOCOL_SRC)

# Client source files
CLIENT_SRC = $(CLIENT_DIR)/src/client.c

# Targets
all: directories server client

directories:
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJ_DIR)
	mkdir -p database

server: $(ALL_SERVER_SRC)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/server $(ALL_SERVER_SRC)

client: $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/client $(CLIENT_SRC)

clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)

clean-all: clean
	rm -f database/users.txt

run-server:
	./$(BIN_DIR)/server

run-client:
	./$(BIN_DIR)/client

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: all

.PHONY: all clean clean-all directories run-server run-client debug
