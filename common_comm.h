#ifndef COMMON_COMM_H_
#define COMMON_COMM_H_

#include <cinttypes>
#include <fstream>
#include <string>

namespace IO {
/** Enums representing the server responses. */
enum class Response { OK, Error };

/** Interface that the communication objects must implement. */
class Comm {
   public:
    Comm(){};
    virtual ~Comm(){};

    virtual Comm& operator<<(uint8_t c) = 0;
    virtual Comm& operator<<(Response r) = 0;
    virtual Comm& operator<<(uint32_t i) = 0;
    virtual Comm& operator<<(std::size_t i) = 0;
    virtual Comm& operator<<(const char* s) = 0;
    virtual Comm& operator<<(const std::string& s) = 0;
    virtual Comm& operator<<(std::ifstream& s) = 0;

    virtual Comm& operator>>(uint8_t& c) = 0;
    virtual Comm& operator>>(Response& r) = 0;
    virtual Comm& operator>>(uint32_t& i) = 0;
    virtual Comm& operator>>(std::string& s) = 0;
    virtual Comm& operator>>(std::ofstream& s) = 0;

    /* this functions should be implemented by the base class. */
    virtual void write(const void* data, std::size_t size) = 0;
    virtual ssize_t read(void* data, std::size_t size) = 0;
};
}  // namespace IO

#endif
