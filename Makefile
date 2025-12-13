# Makefile for Client-Server Network Programming Project

CC = gcc
CFLAGS = -Wall -pthread
SERVER_DIR = server/src
CLIENT_DIR = client/src
BIN_DIR = bin

# Targets
all: directories server client

directories:
	mkdir -p $(BIN_DIR)
	mkdir -p database

server: $(SERVER_DIR)/server.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/server $(SERVER_DIR)/server.c

client: $(CLIENT_DIR)/client.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/client $(CLIENT_DIR)/client.c

clean:
	rm -rf $(BIN_DIR)
	rm -f database/users.txt

run-server:
	./$(BIN_DIR)/server

run-client:
	./$(BIN_DIR)/client

.PHONY: all clean directories run-server run-client
