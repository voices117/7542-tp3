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
        handler(std::move(client));

        /* calls the client handler in a new thread */
        // TODO: make multithreaded
        // auto action = [client, handler] { handler(client); };
        // this->handlers.push_back(std::thread(action));
    }

   private:
    /** Internal server socket. */
    TP3::Socket socket;
    /** Internal array of client handlers */
    std::vector<std::thread> handlers;
};
}  // namespace Server

#endif
