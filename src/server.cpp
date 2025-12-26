#include <iostream>
#include "my_socket_lib.hpp"
#include "logger.hpp"


int main (void)
{
    Server server;
    SysLogger logger("HW_Server");
    int ret{};
    int client_fd{};

    ret = server.connect_to_port(3500, 25);
    if (ret < 0) {
        logger.error("Failed to start server");
        return 1;
    }

    logger.info("Server listening on port 3500...");

    while (1)
    {
        client_fd = server.wait_for_connection();
        if (client_fd <= 0) {
            logger.error("Failed to connect to a client");
            continue;
        }

        logger.info("Client connected");

        std::string data_buffer{};
        ret = server.receive_request(client_fd, data_buffer);

        if (ret < 0) {
            logger.error("Failed to recieve data from client.");
            close(client_fd);
            continue;
        }

        logger.info("Received: " + data_buffer);

        std::string data_sent = "Hello from Server";
        ret = server.send_data(client_fd, data_buffer);
        if (ret == -1) {
            logger.error("Failed to send data to server");
            close(client_fd);
            continue;
        }

        logger.info("Response sent to client");
        close(client_fd);
    }


    return 0;
}