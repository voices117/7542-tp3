#include "common_socket.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include "common_error.h"

/** use POSIX extensions */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

TP3::Socket::Socket() {
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd == -1) {
        throw TP3::Error{"Error al crear socket: %s", strerror(errno)};
    }
}

TP3::Socket::Socket(Socket&& other) {
    std::swap(this->fd, other.fd);
}

TP3::Socket::~Socket() {
    /* checks if the socket has to be released */
    if (this->fd < 0) {
        return;
    }

    shutdown(this->fd, SHUT_RDWR);
    close(this->fd);
    this->fd = -1;
}

/**
 * @brief Construct a new Socket directly with the given file descriptor.
 *
 * @param fd File descriptor.
 */
TP3::Socket::Socket(int fd) {
    this->fd = fd;
}

/**
 * @brief Binds the socket to the given service (or port number as a string).
 * The socket will be set to passive mode to work as a server.
 *
 * @param service Service/port to bind the socket to.
 */
void TP3::Socket::bind(const std::string& service) {
    /* sets the hints */
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* ptr;
    int rv = getaddrinfo(NULL, service.c_str(), &hints, &ptr);
    if (rv != 0) {
        freeaddrinfo(ptr);
        throw TP3::Error{"getaddrinfo: %s", gai_strerror(rv)};
    }

    /* avoids time waits */
    int val = 1;
    if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) ==
        -1) {
        freeaddrinfo(ptr);
        throw TP3::Error{"setsockopt: %s", strerror(errno)};
    }

    /* binds the socket */
    if (::bind(this->fd, ptr->ai_addr, ptr->ai_addrlen) == -1) {
        freeaddrinfo(ptr);
        throw TP3::Error{"bind: %s", strerror(errno)};
    }

    freeaddrinfo(ptr);
}

/**
 * @brief Connects the socket (active socket) to the given address and
 * service/port.
 *
 * @param address Address to connect to.
 * @param service The service (or port as a string).
 */
void TP3::Socket::connect(const std::string& address,
                          const std::string& service) {
    /* sets the hints for the required protocol */
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;

    /* gets the address info */
    struct addrinfo* result;
    int rv = getaddrinfo(address.c_str(), service.c_str(), &hints, &result);
    if (rv != 0) {
        throw TP3::Error{"getaddrinfo: %s", gai_strerror(rv)};
    }

    /* searches through the addresses list */
    for (struct addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        /* does the actual connection */
        if (::connect(this->fd, ptr->ai_addr, ptr->ai_addrlen) == 0) {
            freeaddrinfo(result);
            return;
        }
    }

    /* if it reached this point, it means that it couldn't connect */
    freeaddrinfo(result);
    throw TP3::Error{"No se pudo conectar el socket"};
}

/**
 * @brief Blocks until a client connects to the socket.
 * The socket must have been binded before calling this function.
 *
 * @return Client socket.
 */
TP3::Socket TP3::Socket::accept() {
    struct sockaddr_in cli_addr = {0};
    size_t addr_size = sizeof(struct sockaddr_in);

    int client_fd =
        ::accept(this->fd, (struct sockaddr*)&cli_addr, (socklen_t*)&addr_size);
    if (client_fd == -1) {
        throw TP3::Error{"accept: %s", strerror(errno)};
    }

    /* creates and returns the client socket */
    return TP3::Socket{client_fd};
}

/**
 * @brief Sends a chunk of data through the socket.
 *
 * @param data Pointer to the data buffer.
 * @param size Size of the buffer.
 */
void TP3::Socket::write(const void* data, std::size_t size) {
    /* casts the data buffer to a byte array */
    auto out = static_cast<const unsigned char*>(data);
    size_t total_bytes_written = 0;
    do {
        ssize_t bytes_written = send(this->fd, out + total_bytes_written,
                                     size - total_bytes_written, MSG_NOSIGNAL);
        if (bytes_written <= 0) {
            throw TP3::Error{"send: %s", strerror(errno)};
        }

        total_bytes_written += bytes_written;
    } while (total_bytes_written < size);
}

/**
 * @brief Reads a chunk of bytes from the socket.
 *
 * @param data Pointer to the output buffer.
 * @param size Bytes to read.
 */
void TP3::Socket::read(void* data, std::size_t size) {
    ssize_t bytes_read = recv(this->fd, data, size, MSG_WAITALL);
    if (bytes_read < 0) {
        throw TP3::Error{"recv: %s", strerror(errno)};
    }
}

/**
 * @brief Insertion operator for the socket class.
 *
 * @param i Unsigned integer to write (in network order).
 * @return Socket.
 */
TP3::Socket& TP3::Socket::operator<<(uint32_t i) {
    /* writes the integer in network byte order */
    auto output = htonl(i);
    this->write(&output, sizeof(output));
    return *this;
}

/**
 * @brief Insertion operator for the socket class.
 *
 * @param s C string to write.
 * @return Socket.
 *
 * @note The string is written as a 4 byte value with the length and the content
 * after that.
 */
TP3::Socket& TP3::Socket::operator<<(const char* s) {
    /* writes the string length first */
    uint32_t len = strlen(s);
    *this << len;

    /* writes the actual string content */
    this->write(s, len);
    return *this;
}

/**
 * @brief Insertion operator for the socket class.
 *
 * @param s String to write.
 * @return Socket.
 *
 * @note The string is written as a 4 byte value with the length and the content
 * after that.
 */
TP3::Socket& TP3::Socket::operator<<(const std::string& s) {
    return *this << s.c_str();
}

/**
 * @brief Extraction operator for the socket class.
 *
 * @param i To reference to the integer where the read value is written
 * (converted to host order).
 * @return Socket.
 */
TP3::Socket& TP3::Socket::operator>>(uint32_t& i) {
    /* reads from the socket in network order */
    uint32_t tmp;
    this->read(&tmp, sizeof(tmp));

    /* converts to host order and sets the output */
    i = ntohl(tmp);
    return *this;
}

/**
 * @brief Extraction operator for the socket class.
 *
 * @param s String to be read.
 * @return Socket.
 */
TP3::Socket& TP3::Socket::operator>>(std::string& s) {
    /* first it reads the string length as a 4 byte value in host order */
    uint32_t len;
    *this >> len;

    /* preallocates the required space into the string */
    s.resize(len);

    /* gets the actual content */
    this->read(&s.front(), len);
    return *this;
}
