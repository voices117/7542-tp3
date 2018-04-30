#ifndef VERSIONER_H_
#define VERSIONER_H_

#include <string>
#include <vector>
#include "common_comm.h"
#include "common_error.h"
#include "common_socket.h"

namespace Client {
class Versioner {
   public:
    Versioner(IO::Comm& comm);
    ~Versioner();

    void push(const std::string& file_name, const std::string& hash);
    void pull(const std::string& tag);
    void tag(const std::string& tag, std::vector<std::string>& hashes);

   private:
    /** Internal socket used to communicate with the server. */
    IO::Comm& comm;
};
}  // namespace Client

#endif
