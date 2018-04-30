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
    Versioner(std::ifstream& file);
    ~Versioner();

    Versioner& operator=(Versioner& other) = delete;
    void operator()(TP3::Socket&& client);

    void push(IO::Comm& comm);
    void pull(IO::Comm& comm);
    void tag(IO::Comm& comm);

   private:
    TP3::FileIndex file_index;
    TagIndex tag_index;

    std::mutex mutex;
    std::condition_variable cv;

    int readers{true};
};
}  // namespace Server

#endif
