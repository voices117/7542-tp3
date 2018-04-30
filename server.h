#ifndef SERVER_H_
#define SERVER_H_

#include <thread>
#include <vector>
#include "common_socket.h"

namespace Server {
class Server {
   public:
    Server(const std::string& port);
    ~Server();

    /** API */
    template <typename Functor>
    void handle_client(Functor& handler) {
        /* blocks until a client connects to the server */
        TP3::Socket client = this->socket.accept();

        /* calls the client handler in a new thread */
        std::thread worker{std::ref(handler), std::move(client)};
        this->handlers.push_back(std::move(worker));
    }

   private:
    /** Internal server socket. */
    TP3::Socket socket;
    /** Internal array of client handlers */
    std::vector<std::thread> handlers;
};
}  // namespace Server

#endif
