#include "hostinfo.h"
#include <netdb.h>     // For 'getnameinfo'
#include <cstring>     // For 'strerror'
#include <iostream>    // For 'cerr'
#include <sstream> 

// Default constructor
hostinfo::hostinfo() : hostname("Unknown") {}

// Constructor that accepts an in_addr
hostinfo::hostinfo(const in_addr& addr) {
    addresses.push_back(addr);
    char buffer[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &addr, buffer, sizeof(buffer))) {
        hostname = buffer;  // Assign the hostname from the address
    } else {
        std::cerr << "Error getting name info: " << strerror(errno) << std::endl;
        hostname = "Invalid Address";  // Handle error case
    }
}

std::string to_string(const hostinfo& info) {
    std::ostringstream oss;
    oss << "Hostinfo: " << info.hostname;  // Use 'hostname' directly
    return oss.str();
}