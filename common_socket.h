#ifndef COMMON_SOCKET_H_
#define COMMON_SOCKET_H_

#include <cinttypes>
#include <string>

namespace TP3 {
class Socket {
   public:
    Socket();
    Socket(Socket&& other);
    ~Socket();

    /** IO */
    void write(const void* data, std::size_t size);
    ssize_t read(void* data, std::size_t size);

    /** Server */
    void bind(const std::string& port);
    void listen();
    Socket accept();

    /** Client */
    void connect(const std::string& address, const std::string& port);

    /** operators */
    Socket& operator=(Socket& other) = delete;

   private:
    /** Private constructor. */
    Socket(int fd);
    /** File descriptor. */
    int fd{-1};
};
}  // namespace TP3

#endif
