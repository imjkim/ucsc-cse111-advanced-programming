#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <functional>
#include <memory> 
using namespace std;

#include <fcntl.h>
#include <limits.h>
#include "sockets.h"

static mutex socket_mtx;

base_socket::base_socket() {
   memset(&socket_addr, 0, sizeof(socket_addr));
}

base_socket::~base_socket() {
   if (socket_fd != CLOSED_FD) close();
}

void base_socket::close() {
   lock_guard<mutex> lock(socket_mtx);
   if (socket_fd != CLOSED_FD) {
      int status = ::close(socket_fd);
      if (status < 0) throw socket_sys_error("close(" + to_string(socket_fd) + ")");
      socket_fd = CLOSED_FD;
   }
}

void base_socket::create() {
   lock_guard<mutex> lock(socket_mtx);
   socket_fd = ::socket(AF_INET, SOCK_STREAM, 0);
   if (socket_fd < 0) throw socket_sys_error("socket");
   int on = 1;
   int status = ::setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
   if (status < 0) throw socket_sys_error("setsockopt");
}

void base_socket::bind(const in_port_t port) {
   lock_guard<mutex> lock(socket_mtx);
   socket_addr.sin_family = AF_INET;
   socket_addr.sin_addr.s_addr = INADDR_ANY;
   socket_addr.sin_port = htons(port);
   int status = ::bind(socket_fd, reinterpret_cast<sockaddr*>(&socket_addr), sizeof(socket_addr));
   if (status < 0) throw socket_sys_error("bind(" + to_string(port) + ")");
}

void base_socket::listen() const {
   lock_guard<mutex> lock(socket_mtx);
   int status = ::listen(socket_fd, SOMAXCONN);
   if (status < 0) throw socket_sys_error("listen");
}

void base_socket::accept(base_socket& socket) const {
   lock_guard<mutex> lock(socket_mtx);
   int addr_length = sizeof(socket.socket_addr);
   socket.socket_fd = ::accept(socket_fd, reinterpret_cast<sockaddr*>(&socket.socket_addr), reinterpret_cast<socklen_t*>(&addr_length));
   if (socket.socket_fd < 0) throw socket_sys_error("accept");
}

ssize_t base_socket::send(const void* buffer, size_t bufsize) {
   lock_guard<mutex> lock(socket_mtx);
   int nbytes = ::send(socket_fd, buffer, bufsize, MSG_NOSIGNAL);
   if (nbytes < 0) throw socket_sys_error("send");
   return nbytes;
}

ssize_t base_socket::recv(void* buffer, size_t bufsize) {
   lock_guard<mutex> lock(socket_mtx);
   memset(buffer, 0, bufsize);
   ssize_t nbytes = ::recv(socket_fd, buffer, bufsize, 0);
   if (nbytes < 0) throw socket_sys_error("recv");
   return nbytes;
}

void base_socket::connect(const std::string& host, const in_port_t port) {
   lock_guard<mutex> lock(socket_mtx);
   struct hostent* hostp = ::gethostbyname(host.c_str());
   if (hostp == NULL) throw socket_h_error("gethostbyname(" + host + ")");
   socket_addr.sin_family = AF_INET;
   socket_addr.sin_port = htons(port);
   socket_addr.sin_addr = *reinterpret_cast<in_addr*>(hostp->h_addr);
   int status = ::connect(socket_fd, reinterpret_cast<sockaddr*>(&socket_addr), sizeof(socket_addr));
   if (status < 0) throw socket_sys_error("connect(" + host + ":" + to_string(port) + ")");
}

void base_socket::set_socket_fd(int fd) {
   lock_guard<mutex> lock(socket_mtx);
   socklen_t addrlen = sizeof(socket_addr);
   int rc = getpeername(fd, reinterpret_cast<sockaddr*>(&socket_addr), &addrlen);
   if (rc < 0) throw socket_sys_error("set_socket_fd(" + to_string(fd) + "): getpeername");
   socket_fd = fd;
   if (socket_addr.sin_family != AF_INET) throw socket_error("address not AF_INET");
}

void base_socket::set_non_blocking(const bool blocking) {
   lock_guard<mutex> lock(socket_mtx);
   int opts = ::fcntl(socket_fd, F_GETFL);
   if (opts < 0) throw socket_sys_error("fcntl");
   if (blocking) opts |= O_NONBLOCK;
   else opts &= ~O_NONBLOCK;
   opts = ::fcntl(socket_fd, F_SETFL, opts);
   if (opts < 0) throw socket_sys_error("fcntl");
}

client_socket::client_socket(string host, in_port_t port) {
   create();
   connect(host, port);
}

server_socket::server_socket(in_port_t port) {
   create();
   bind(port);
   listen();
}

void handle_client(accepted_socket& client) {
    char buffer[1024];

    // Loop to handle communication with the client
    while (true) {
        // Use get_socket_fd() to get the file descriptor
        ssize_t bytes_received = client.recv(buffer, sizeof(buffer));
        
        if (bytes_received < 0) {
            std::cerr << "Error receiving data from client." << std::endl;
            break;  // Exit the loop on error
        } else if (bytes_received == 0) {
            std::cout << "Client disconnected." << std::endl;
            break;  // Exit the loop if the client has disconnected
        }

        // Null-terminate the received data to make it a valid C-string
        buffer[bytes_received] = '\0';
        std::cout << "Received message: " << buffer << std::endl;

        // Process the received message (basic echo server here)
        std::string response = "Echo: " + std::string(buffer);

        // Send a response back to the client using get_socket_fd()
        ssize_t bytes_sent = client.send(response.c_str(), response.size());
        if (bytes_sent < 0) {
            std::cerr << "Error sending data to client." << std::endl;
            break;  // Exit the loop on error
        }
    }

    // Close the client socket when done
    client.close();
}

void server_socket::accept_multiple_clients() {
    vector<thread> threads;
    while (true) {
        accepted_socket client;
        accept(client);

        // Use std::unique_ptr to avoid issues with shared ownership
        auto client_ptr = make_unique<accepted_socket>(std::move(client));

        threads.emplace_back([client_ptr = std::move(client_ptr)]() {
            handle_client(*client_ptr);
        });
    }

    for (auto& th : threads) {
        if (th.joinable()) th.join();
    }
}
std::string to_string(const base_socket& sock) {
    std::ostringstream oss;
    oss << "Socket FD: " << sock.get_socket_fd();
    return oss.str();
}
