#include <iostream>
#include <string>
#include "my_socket_lib.hpp"
#include "logger.hpp"

int main()
{
    // Create client with server hostname and port
    Client client("localhost");
    StdLogger logger("HW_Client");

    // Connect to server
    logger.info("Connecting to server...");
    if (client.connect_to_server(3500) < 0) {
        logger.error("Failed to connect to server");
        return 1;
    }
    
    logger.info("Connected to server!");
    
    // Send message to server
    std::string message = "Hello from C++ client!";
    logger.info("Sending: " + message);
    
    if (client.send_data(message) < 0) {
        logger.error("Failed to send data");
        client.disconnect();
        return 1;
    }
    
    // Receive response from server
    std::string response;
    int bytes = client.receive_data(response);
    
    if (bytes > 0) {
        logger.info("Server response: " + response);
    } else if (bytes == 0) {
        logger.info("Server closed connection");
    } else {
        logger.error("Failed to receive data");
    }
    
    // Clean up
    client.disconnect();
    logger.info("Disconnected from server");

    return 0;
}
