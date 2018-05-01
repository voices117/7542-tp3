#ifndef SERVER_VERSIONER_H_
#define SERVER_VERSIONER_H_

#include <string>
#include "common_comm_socket.h"
#include "common_rw_lock.h"
#include "common_socket.h"
#include "server_file_index.h"
#include "server_tag_index.h"

namespace Server {
class Versioner {
   public:
    Versioner();
    explicit Versioner(Versioner&& other);
    explicit Versioner(const std::string& file_name);
    ~Versioner();

    Versioner& operator=(Versioner& other) = delete;
    void operator()(IO::Socket& client);

    void push(IO::Comm& comm);
    void pull(IO::Comm& comm);
    void tag(IO::Comm& comm);

    void save(std::ofstream& file);

   private:
    FileIndex file_index;
    TagIndex tag_index;

    std::string index_file_name;

    Concurrency::RWLock lock;
};
}  // namespace Server

#endif
