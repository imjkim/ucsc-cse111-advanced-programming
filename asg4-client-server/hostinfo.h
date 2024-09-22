#ifndef HOSTINFO_H
#define HOSTINFO_H

#include <netinet/in.h> // For 'in_addr'
#include <vector>       // For 'std::vector'
#include <string>       // For 'std::string'
#include <netdb.h>      // For 'getnameinfo'
#include <cstring>      // For 'strerror'
#include <iostream>     // For 'std::cerr'
#include <arpa/inet.h>  // For 'inet_ntop'

class hostinfo {
public:
    std::vector<in_addr> addresses; // Stores addresses
    std::string hostname;            // Stores hostname

    // Constructors
    hostinfo();                      // Default constructor
    hostinfo(const in_addr& addr);  // Constructor that accepts an in_addr

    // Destructor
    ~hostinfo() = default;

    // Disable copy semantics
    hostinfo(const hostinfo&) = delete;
    hostinfo& operator=(const hostinfo&) = delete;

    // Enable move semantics if needed
    hostinfo(hostinfo&&) = default;
    hostinfo& operator=(hostinfo&&) = default;
    std::string to_string(const hostinfo& info);

};

#endif // HOSTINFO_H
