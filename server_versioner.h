#ifndef SERVER_VERSIONER_H_
#define SERVER_VERSIONER_H_

#include "common_comm_socket.h"
#include "common_socket.h"
#include "server_file_index.h"
#include "server_tag_index.h"

namespace Server {
class Versioner {
   public:
    Versioner();
    Versioner(Versioner&& other);
    Versioner(const std::string& file_name);
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

    std::mutex mutex;
    std::condition_variable cv;

    std::string index_file_name;

    int readers{true};
};
}  // namespace Server

#endif
