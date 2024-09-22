// $Id: sockets.h,v 1.3 2024-09-20 11:36:22-07 - - $

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <mutex>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

class base_socket {
private:
    static constexpr size_t MAXRECV = 0xFFFF;
    static constexpr int CLOSED_FD = -1;
    int socket_fd {CLOSED_FD}; // Socket file descriptor
    sockaddr_in socket_addr;    // Socket address

protected:
    base_socket();
    base_socket(const base_socket&) = delete;            // Disable copy constructor
    base_socket& operator= (const base_socket&) = delete; // Disable copy assignment operator

    // Move constructor
    base_socket(base_socket&& other) noexcept 
        : socket_fd(other.socket_fd), socket_addr(other.socket_addr) {
        other.socket_fd = CLOSED_FD; // Reset the moved-from socket
    }

    // Move assignment operator
    base_socket& operator=(base_socket&& other) noexcept {
        if (this != &other) {
            close(); // Ensure the current socket is closed if valid
            socket_fd = other.socket_fd;
            socket_addr = other.socket_addr;
            other.socket_fd = CLOSED_FD; // Reset the moved-from socket
        }
        return *this;
    }

    ~base_socket();
    
    void create();
    void bind(const in_port_t port);
    void listen() const;
    void connect(const std::string& host, const in_port_t port);
    void set_socket_fd(int fd);

public:
    void close();
    void accept(base_socket& sock) const;
    int get_socket_fd() const { return socket_fd; }
    ssize_t send(const void* buffer, size_t bufsize);
    ssize_t recv(void* buffer, size_t bufsize);
    void set_non_blocking(const bool);
    
    friend std::string to_string(const base_socket& sock);
};
std::string to_string(const base_socket& sock);

class accepted_socket : public base_socket {
   public:
      accepted_socket() {}
      accepted_socket(accepted_socket&& other) noexcept : base_socket(std::move(other)) {}
      accepted_socket& operator=(accepted_socket&& other) noexcept {
         base_socket::operator=(std::move(other));
         return *this;
      }
};
class client_socket : public base_socket {
public:
    client_socket(string host, in_port_t port);
};

class server_socket : public base_socket {
public:
    server_socket(in_port_t port);
    void accept_multiple_clients();
};

class socket_error : public runtime_error {
public:
    explicit socket_error(const string& what) : runtime_error(what) {}
};

class socket_sys_error : public socket_error {
public:
    int sys_errno;
    explicit socket_sys_error(const string& what) :
        socket_error(what + ": " + strerror(errno)),
        sys_errno(errno) {}
};

class socket_h_error : public socket_error {
public:
    int host_errno;
    explicit socket_h_error(const string& what) :
        socket_error(what + ": " + hstrerror(h_errno)),
        host_errno(h_errno) {}
};

class hostinfo {
public:
    const string hostname;
    const vector<string> aliases;
    const vector<in_addr> addresses;
    hostinfo();
    hostinfo(hostent*);
    hostinfo(const string& hostname);
    hostinfo(const in_addr& ipv4_addr);
    friend string to_string(const hostinfo&);
};

string localhost();
string to_string(const in_addr& ipv4_addr);

#endif // __SOCKET_H__
