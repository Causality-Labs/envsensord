# Compiler and flags
CXX = arm-linux-gnueabihf-g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinc
LDFLAGS = 

# Directories
SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj
BIN_DIR = bin

# Source files
LIB_SRC = $(SRC_DIR)/my_socket_lib.cpp
BME280_SRC = $(SRC_DIR)/bme280.cpp
CLIENT_SRC = $(SRC_DIR)/client.cpp
SERVER_SRC = $(SRC_DIR)/server.cpp
SSNP_SRC   = $(SRC_DIR)/SSNP.cpp
CLP_SRC = $(SRC_DIR)/CommandLineParser.cpp

# Object files
LIB_OBJ = $(OBJ_DIR)/my_socket_lib.o
BME280_OBJ = $(OBJ_DIR)/bme280.o
CLIENT_OBJ = $(OBJ_DIR)/client.o
SERVER_OBJ = $(OBJ_DIR)/server.o
SSNP_OBJ   = $(OBJ_DIR)/SSNP.o
CLP_OBJ    = $(OBJ_DIR)/CommandLineParser.o

# Executables
CLIENT_BIN = $(BIN_DIR)/client
SERVER_BIN = $(BIN_DIR)/server

# Default target: build everything
all: server client

# Create necessary directories
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

# Target: build server
server: $(SERVER_BIN)

# Target: build client
client: $(CLIENT_BIN)

# Build client executable
$(CLIENT_BIN): $(CLIENT_OBJ) $(LIB_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Build server executable
$(SERVER_BIN): $(SERVER_OBJ) $(BME280_OBJ) $(SSNP_OBJ) $(CLP_OBJ) $(LIB_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile library object file
$(LIB_OBJ): $(LIB_SRC) $(INC_DIR)/my_socket_lib.hpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile BME280 object file
$(BME280_OBJ): $(BME280_SRC) $(INC_DIR)/bme280.hpp $(INC_DIR)/logger.hpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile SSNP object file
$(SSNP_OBJ): $(SSNP_SRC) $(INC_DIR)/SSNP.hpp $(INC_DIR)/bme280.hpp $(INC_DIR)/logger.hpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile CommandLineParser object file
$(CLP_OBJ): $(CLP_SRC) $(INC_DIR)/CommandLineParser.hpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile client object file
$(CLIENT_OBJ): $(CLIENT_SRC) $(INC_DIR)/my_socket_lib.hpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile server object file
$(SERVER_OBJ): $(SERVER_SRC) $(INC_DIR)/my_socket_lib.hpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Rebuild everything
rebuild: clean all

.PHONY: all server client clean rebuild
