#include <iostream>
#include <string>
#include "my_socket_lib.hpp"

int main()
{
    // Create client with server hostname and port
    Client client("localhost");
    
    // Connect to server
    std::cout << "Connecting to server..." << std::endl;
    if (client.connect_to_server(3500) < 0) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }
    
    std::cout << "Connected to server!" << std::endl;
    
    // Send message to server
    std::string message = "Hello from C++ client!";
    std::cout << "Sending: " << message << std::endl;
    
    if (client.send_data(message) < 0) {
        std::cerr << "Failed to send data" << std::endl;
        client.disconnect();
        return 1;
    }
    
    // Receive response from server
    std::string response;
    int bytes = client.receive_data(response);
    
    if (bytes > 0) {
        std::cout << "Server response: " << response << std::endl;
    } else if (bytes == 0) {
        std::cout << "Server closed connection" << std::endl;
    } else {
        std::cerr << "Failed to receive data" << std::endl;
    }
    
    // Clean up
    client.disconnect();
    std::cout << "Disconnected from server" << std::endl;

    return 0;
}
