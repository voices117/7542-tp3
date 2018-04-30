#ifndef SERVER_H_
#define SERVER_H_

#include <atomic>
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

    /** Receives a functor that expects a parameter Socket as an rvalue.
     *  The functor is executed when a client connects and is passed that
     *  client's socket.
     */
    void handle_client(Functor& handler) {
        /* blocks until a client connects to the server */
        IO::Socket client = this->socket.accept();

        /* calls the client handler in a new thread */
        std::thread worker{std::ref(handler), std::move(client)};
        this->handlers.push_back(std::move(worker));
    }

   private:
    /** Internal server socket. */
    IO::Socket socket;
    /** Internal array of client handlers */
    std::vector<std::thread> handlers;
    /** Thread that waits for the exit signal. */
    std::thread exit_thread;

    void exit_handler();
};
}  // namespace Server

#endif
