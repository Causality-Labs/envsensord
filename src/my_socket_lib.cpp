#include "my_socket_lib.hpp"
#include <vector>

Server::Server() 
: socket_fd(-1), portno(3500) {
}


Server::~Server() {
    if (socket_fd >= 0) {
        close(socket_fd);
    }
}

int Server::connect_to_port(int port, int maxnpending)
{
    struct addrinfo hints {};
    struct addrinfo *res = nullptr;
    struct addrinfo *res_pointer = nullptr;
    int ret = 0;

    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    std::string portno_string = std::to_string(port);
    portno = port;

    ret = getaddrinfo(nullptr, portno_string.c_str(), &hints, &res);
    if (ret != 0) {
        std::cerr << __FILE__ << " @ " << __LINE__ 
            << ": Could not get addrinfo Error: " 
            << gai_strerror(ret) << std::endl;
        return -1;
    }

        // Try each address until we successfully bind and listen
    for (res_pointer = res; res_pointer != nullptr; res_pointer = res_pointer->ai_next) {
        socket_fd = socket(res_pointer->ai_family, 
                          res_pointer->ai_socktype, 
                          res_pointer->ai_protocol);
        if (socket_fd < 0)
            continue;

        // Allow socket reuse
        int one = 1;
        ret = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        if (ret != 0) {
            std::cerr << __FILE__ << " @ " << __LINE__ 
                      << ": Failed to setsockopt Error: " 
                      << std::strerror(errno) << std::endl;
            close(socket_fd);
            freeaddrinfo(res);
            return -1;
        }

        // Allow IPv4 connections on IPv6 socket
        int off = 0;
        ret = setsockopt(socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof(off));
        if (ret != 0) {
            std::cerr << __FILE__ << " @ " << __LINE__ 
                      << ": Failed to setsockopt Error: " 
                      << std::strerror(errno) << std::endl;
            close(socket_fd);
            freeaddrinfo(res);
            return -1;
        }

        // Bind and listen
        ret = bind(socket_fd, res_pointer->ai_addr, res_pointer->ai_addrlen);
        if (ret == 0) {
            ret = listen(socket_fd, maxnpending);
            if (ret == 0)
                break;  // Success!
        }

        close(socket_fd);
        socket_fd = -1;
    }

    freeaddrinfo(res);
    
    if (res_pointer == nullptr) {
        std::cerr << __FILE__ << " @ " << __LINE__ 
                  << ": Failed to bind server Error: " 
                  << std::strerror(errno) << std::endl;
        return -1;
    }

    max_pending_clients = maxnpending;

    return socket_fd;
}

int Server::wait_for_connection()
{
    struct sockaddr_storage client_addr{};
    socklen_t client_len = sizeof(client_addr);
    
    int client_fd = accept(socket_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    
    if (client_fd < 0) {
        std::cerr << __FILE__ << " @ " << __LINE__ 
                  << ": Failed to accept connection Error: " 
                  << std::strerror(errno) << std::endl;
        return -1;
    }
    
    return client_fd;
}

int Server::receive_request(int client_fd, std::string& data)
{
    constexpr size_t BUFFER_SIZE = 1024;
    data.resize(BUFFER_SIZE);

    ssize_t bytes_received = recv(client_fd, data.data(), data.size(), 0);

    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            std::cout << "Client disconnected" << std::endl;
        } else {
            std::cerr << __FILE__ << " @ " << __LINE__ 
                      << ": Failed to receive data Error: " 
                      << std::strerror(errno) << std::endl;
        }
        data.clear();
        return -1;
    }

    data.resize(bytes_received);
    return 0;
}

int Server::send_data(int client_fd, const std::string& data)
{
    ssize_t bytes_sent = send(client_fd, data.c_str(), data.length(), 0);
    
    if (bytes_sent < 0) {
        std::cerr << __FILE__ << " @ " << __LINE__ 
                  << ": Failed to send data Error: " 
                  << std::strerror(errno) << std::endl;
        return -1;
    }
    
    if (static_cast<size_t>(bytes_sent) < data.length()) {
        std::cerr << "Warning: Only sent " << bytes_sent 
                  << " of " << data.length() << " bytes" << std::endl;
    }
    
    return bytes_sent;
}

void Server::close_client(int client_fd)
{
    if (client_fd >= 0) {
        close(client_fd);
    }
}

void Server::stop_listening()
{
    if (socket_fd >= 0) {
        close(socket_fd);
        socket_fd = -1;
    }
}

Client::Client() 
    : socket_fd(-1), portno(3500), hostname("") {
}

Client::Client(const std::string& hostname)
    : socket_fd(-1), portno(3500), hostname(hostname) {
}

Client::~Client() {
    if (socket_fd >= 0) {
        close(socket_fd);
    }
}

int Client::connect_to_server(int port) {
    struct addrinfo hints{};
    struct addrinfo *res = nullptr;
    struct addrinfo *res_pointer = nullptr;
    int ret{};
    
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    std::string portno_string = std::to_string(port);
    
    ret = getaddrinfo(hostname.c_str(), portno_string.c_str(), &hints, &res);
    portno = port;

    if (ret != 0) {
        std::cerr << __FILE__ << " @ " << __LINE__ 
                  << ": Could not get addrinfo Error: " 
                  << gai_strerror(ret) << std::endl;
        return -1;
    }
    
    for (res_pointer = res; res_pointer != nullptr; res_pointer = res_pointer->ai_next) {
        socket_fd = socket(res_pointer->ai_family, 
                          res_pointer->ai_socktype, 
                          res_pointer->ai_protocol);
        if (socket_fd < 0)
            continue;
        
        ret = connect(socket_fd, res_pointer->ai_addr, res_pointer->ai_addrlen);
        if (ret == 0)
            break;  // Success!
        
        close(socket_fd);
        socket_fd = -1;
    }
    
    freeaddrinfo(res);
    
    if (res_pointer == nullptr) {
        std::cerr << __FILE__ << " @ " << __LINE__ 
                  << ": Failed to connect to server Error: " 
                  << std::strerror(errno) << std::endl;
        return -1;
    }
    
    return socket_fd;
}

int Client::send_data(const std::string& data) {
    ssize_t bytes_sent = send(socket_fd, data.c_str(), data.length(), 0);
    
    if (bytes_sent < 0) {
        std::cerr << __FILE__ << " @ " << __LINE__ 
                  << ": Failed to send data Error: " 
                  << std::strerror(errno) << std::endl;
        return -1;
    }
    
    return bytes_sent;
}

int Client::receive_data(std::string& data) {
    constexpr size_t BUFFER_SIZE = 1024;
    data.resize(BUFFER_SIZE);
    
    ssize_t bytes_received = recv(socket_fd, data.data(), data.size(), 0);
    
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            std::cout << "Server disconnected" << std::endl;
        } else {
            std::cerr << __FILE__ << " @ " << __LINE__ 
                      << ": Failed to receive data Error: " 
                      << std::strerror(errno) << std::endl;
        }
        data.clear();
        return bytes_received;
    }
    
    data.resize(bytes_received);
    return bytes_received;
}

void Client::disconnect()
{
    if (socket_fd >= 0) {
        close(socket_fd);
    }
}