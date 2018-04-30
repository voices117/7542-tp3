#include "common_comm_socket.h"
#include <arpa/inet.h>
#include <algorithm>
#include <cstring>
#include "common_error.h"

IO::CommSocket::CommSocket(const std::string& address,
                           const std::string& service) {
    this->socket.connect(address, service);
}

IO::CommSocket::CommSocket(IO::Socket&& socket) : socket(std::move(socket)) {
}

IO::CommSocket::~CommSocket() {
}

/**
 * @brief Writes a chunk of bytes into the internal socket.
 *
 * @param data Pointer to the data buffer.s
 * @param size Size of the input buffer.
 */
void IO::CommSocket::write(const void* data, std::size_t size) {
    this->socket.write(data, size);
}

/**
 * @brief Reads a chunk of bytes from the internal socket.
 *
 * @param data Output buffer.
 * @param size Bytes to read.
 * @return ssize_t Bytes actually read or negative value in case of error.
 */
ssize_t IO::CommSocket::read(void* data, std::size_t size) {
    return this->socket.read(data, size);
}

/**
 * @brief Insertion operator for the Comm class.
 *
 * @param c Unsigned byte to write (in network order).
 * @return self.
 */
IO::Comm& IO::CommSocket::operator<<(uint8_t c) {
    this->write(&c, sizeof(c));
    return *this;
}

/**
 * @brief Insertion operator for the Comm class.
 *
 * @param r Response to write.
 * @return self.
 */
IO::Comm& IO::CommSocket::operator<<(IO::Response r) {
    switch (r) {
        case IO::Response::OK:
            return *this << static_cast<uint8_t>(1);
        case IO::Response::Error:
            return *this << static_cast<uint8_t>(0);
        default:
            throw Error::Error{"Unexpected response type"};
    }

    return *this;
}

/**
 * @brief Insertion operator for the Comm class.
 *
 * @param i Unsigned integer to write (in network order).
 * @return self.
 */
IO::Comm& IO::CommSocket::operator<<(uint32_t i) {
    /* writes the integer in network byte order */
    auto output = htonl(i);
    this->write(&output, sizeof(output));
    return *this;
}

/**
 * @brief Insertion operator for the Comm class.
 *
 * @param i size_t to write (in network order).
 * @return self.
 */
IO::Comm& IO::CommSocket::operator<<(std::size_t i) {
    if (i > __UINT32_MAX__) {
        throw Error::Error{"size_t demasiado grande"};
    }
    return *this << static_cast<uint32_t>(i);
}

/**
 * @brief Insertion operator for the Comm class.
 *
 * @param s C string to write.
 * @return self.
 *
 * @note The string is written as a 4 byte value with the length and the content
 * after that.
 */
IO::Comm& IO::CommSocket::operator<<(const char* s) {
    /* writes the string length first */
    uint32_t len = strlen(s);
    *this << len;

    /* writes the actual string content */
    this->write(s, len);
    return *this;
}

/**
 * @brief Insertion operator for the Comm class.
 *
 * @param s String to write.
 * @return self.
 *
 * @note The string is written as a 4 byte value with the length and the content
 * after that.
 */
IO::Comm& IO::CommSocket::operator<<(const std::string& s) {
    return *this << s.c_str();
}

/**
 * @brief Extraction operator for the Comm class.
 *
 * @param c To reference to the byte where the read value is written
 * (converted to host order).
 * @return self.
 */
IO::Comm& IO::CommSocket::operator>>(uint8_t& c) {
    if (this->read(&c, sizeof(c)) != sizeof(c)) {
        throw IO::CommError{"Error en la lectura de u8"};
    }
    return *this;
}

/**
 * @brief Extraction operator for the Comm class.
 *
 * @param r Response to write.
 * @return self.
 */
IO::Comm& IO::CommSocket::operator>>(IO::Response& r) {
    uint8_t code;
    *this >> code;

    switch (code) {
        case 1:
            r = IO::Response::OK;
            break;
        case 0:
            r = IO::Response::Error;
            break;
        default:
            throw Error::Error{"Valor de respuesta invalido"};
    }

    return *this;
}

/**
 * @brief Extraction operator for the Comm class.
 *
 * @param i To reference to the integer where the read value is written
 * (converted to host order).
 * @return self.
 */
IO::Comm& IO::CommSocket::operator>>(uint32_t& i) {
    /* reads from the Comm in network order */
    uint32_t tmp;
    if (this->read(&tmp, sizeof(tmp)) != sizeof(i)) {
        throw IO::CommError{"Error en la lectura de u32"};
    }

    /* converts to host order and sets the output */
    i = ntohl(tmp);
    return *this;
}

/**
 * @brief Extraction operator for the Comm class.
 *
 * @param s String to be read.
 * @return self.
 */
IO::Comm& IO::CommSocket::operator>>(std::string& s) {
    /* first it reads the string length as a 4 byte value in host order */
    uint32_t len;
    *this >> len;

    /* preallocates the required space into the string */
    s.resize(len);

    /* gets the actual content */
    if (this->read(&s.front(), len) != len) {
        throw IO::CommError{"Error en la lectura de string"};
    }
    return *this;
}

/**
 * @brief Sends a file through the socket.
 *
 * @param file An opened stream.
 * @return self.
 */
IO::Comm& IO::CommSocket::operator<<(std::ifstream& file) {
    /* gets the file size */
    file.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize length = file.gcount();
    uint32_t fsize = length;

    /* resets so it can later read the contents */
    file.clear();
    file.seekg(0, std::ios_base::beg);

    /* sends the whole file size first */
    *this << fsize;

    /* sends the file contents in chunks */
    char buffer[1024];
    while (true) {
        file.read(buffer, sizeof(buffer));
        std::streamsize bytes_read = file.gcount();
        if (bytes_read <= 0) {
            break;
        }
        this->write(buffer, bytes_read);
    }

    return *this;
}

/**
 * @brief Receives a file from the socket.
 *
 * @param file An opened file stream to write the data.
 * @return self.
 */
IO::Comm& IO::CommSocket::operator>>(std::ofstream& file) {
#define BUFFER_SIZE ((uint32_t)1024)
    uint32_t size;
    *this >> size;

    uint32_t total_bytes_read = 0;

    /* reads the file by chunks */
    while (total_bytes_read < size) {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read =
            this->read(buffer, std::min(size - total_bytes_read, BUFFER_SIZE));

        file.write(buffer, bytes_read);
        total_bytes_read += bytes_read;
    }

    return *this;
}
