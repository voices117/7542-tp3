#ifndef COMMON_COMM_SOCKET_H_
#define COMMON_COMM_SOCKET_H_

#include "common_comm.h"
#include "common_socket.h"

namespace IO {
class CommSocket : public Comm {
   public:
    CommSocket(const std::string& address, const std::string& service);
    CommSocket(Socket&& socket);
    ~CommSocket();

    /* overrides */
    virtual void write(const void* data, std::size_t size) override;
    virtual ssize_t read(void* data, std::size_t size) override;
    virtual Comm& operator<<(uint8_t c) override;
    virtual Comm& operator<<(Response r) override;
    virtual Comm& operator<<(uint32_t i) override;
    virtual Comm& operator<<(std::size_t i) override;
    virtual Comm& operator<<(const char* s) override;
    virtual Comm& operator<<(const std::string& s) override;
    virtual Comm& operator<<(std::ifstream& file) override;

    virtual Comm& operator>>(uint8_t& c) override;
    virtual Comm& operator>>(Response& r) override;
    virtual Comm& operator>>(uint32_t& i) override;
    virtual Comm& operator>>(std::string& s) override;
    virtual Comm& operator>>(std::ofstream& file) override;

   private:
    Socket socket;
};
}  // namespace IO

#endif
