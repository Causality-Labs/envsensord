#ifndef MY_SOCKET_LIB_HPP
#define MY_SOCKET_LIB_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <cstring> 


class Server {
    private:
        int socket_fd;
        int portno;
        int max_pending_clients;

    public:
        Server();
        ~Server();

        int connect_to_port(int port, int maxnpending);
        int wait_for_connection();
        int send_data(int client_fd, const std::string& data);
        int receive_request(int client_fd, std::string& data);
        void close_client(int client_fd);
        void stop_listening();
};

class Client {
    private:
        int socket_fd;
        int portno;
        std::string hostname;

    public:
        Client();
        Client(const std::string& hostname);
        ~Client();
        
        int connect_to_server(int port);
        int send_data(const std::string& data);
        int receive_data(std::string& data);
        void disconnect();
};

#endif // MY_SOCKET_LIB_HPP