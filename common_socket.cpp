#include "common_socket.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <string>
#include "common_error.h"

/** use POSIX extensions */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

IO::Socket::Socket() {
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd == -1) {
        throw Error::Error{"Error al crear socket: %s", strerror(errno)};
    }
}

IO::Socket::Socket(Socket&& other) {
    std::swap(this->fd, other.fd);
}

IO::Socket::~Socket() {
    /* checks if the socket has to be released */
    if (this->fd < 0) {
        return;
    }
    this->shutdown();
    close(this->fd);
    this->fd = -1;
}

/**
 * @brief Construct a new Socket directly with the given file descriptor.
 *
 * @param fd File descriptor.
 */
IO::Socket::Socket(int fd) {
    this->fd = fd;
}

/**
 * @brief Binds the socket to the given service (or port number as a string).
 *
 * @param port Service/port to bind the socket to.
 */
void IO::Socket::bind(const std::string& port) {
    /* sets the hints */
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* ptr;
    int rv = getaddrinfo(NULL, port.c_str(), &hints, &ptr);
    if (rv != 0) {
        freeaddrinfo(ptr);
        throw Error::Error{"getaddrinfo: %s", gai_strerror(rv)};
    }

    /* avoids time waits */
    int val = 1;
    if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) ==
        -1) {
        freeaddrinfo(ptr);
        throw Error::Error{"setsockopt: %s", strerror(errno)};
    }

    /* binds the socket */
    if (::bind(this->fd, ptr->ai_addr, ptr->ai_addrlen) == -1) {
        freeaddrinfo(ptr);
        throw Error::Error{"bind: %s", strerror(errno)};
    }

    freeaddrinfo(ptr);
}

/**
 * @brief Sets a socket to passive mode.
 */
void IO::Socket::listen() {
    if (::listen(this->fd, 10) == -1) {
        throw Error::Error{"listen: %s", strerror(errno)};
    }
}

/**
 * @brief Connects the socket (active socket) to the given address and
 * service/port.
 *
 * @param address Address to connect to.
 * @param port The service (or port as a string).
 */
void IO::Socket::connect(const std::string& address, const std::string& port) {
    /* sets the hints for the required protocol */
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;

    /* gets the address info */
    struct addrinfo* result;
    int rv = getaddrinfo(address.c_str(), port.c_str(), &hints, &result);
    if (rv != 0) {
        throw Error::Error{"getaddrinfo: %s", gai_strerror(rv)};
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
    throw Error::Error{"No se pudo conectar el socket"};
}

/**
 * @brief Blocks until a client connects to the socket.
 * The socket must have been binded before calling this function.
 *
 * @return Client socket.
 */
IO::Socket IO::Socket::accept() {
    struct sockaddr_in cli_addr = {0};
    size_t addr_size = sizeof(struct sockaddr_in);

    int client_fd =
        ::accept(this->fd, (struct sockaddr*)&cli_addr, (socklen_t*)&addr_size);
    if (client_fd == -1) {
        if (errno == EINVAL) {
            /* the socket was probably shutdown to interrupt the accept */
            throw IO::Interrupted{};
        } else {
            throw Error::Error{"accept: %s", strerror(errno)};
        }
    }

    /* creates and returns the client socket */
    return IO::Socket{client_fd};
}

/**
 * @brief Forces a socket shutdown.
 *
 */
void IO::Socket::shutdown() {
    ::shutdown(this->fd, SHUT_RDWR);
}

/**
 * @brief Sends a chunk of data through the socket.
 *
 * @param data Pointer to the data buffer.
 * @param size Size of the buffer.
 */
void IO::Socket::write(const void* data, std::size_t size) {
    /* casts the data buffer to a byte array */
    auto out = static_cast<const unsigned char*>(data);
    size_t total_bytes_written = 0;
    do {
        ssize_t bytes_written = send(this->fd, out + total_bytes_written,
                                     size - total_bytes_written, MSG_NOSIGNAL);
        if (bytes_written <= 0) {
            throw Error::Error{"send: %s", strerror(errno)};
        }

        total_bytes_written += bytes_written;
    } while (total_bytes_written < size);
}

/**
 * @brief Reads a chunk of bytes from the socket.
 *
 * @param data Pointer to the output buffer.
 * @param size Bytes to read.
 * @return Bytes written into the buffer.
 */
ssize_t IO::Socket::read(void* data, std::size_t size) {
    ssize_t bytes_read = recv(this->fd, data, size, MSG_WAITALL);
    if (bytes_read < 0) {
        throw Error::Error{"recv: %s", strerror(errno)};
    }
    return bytes_read;
}
