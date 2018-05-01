#ifndef SERVER_H_
#define SERVER_H_

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include "common_socket.h"

namespace Server {
/**
 * @brief This class is meant for internal use.
 * It wraps handler objects inside the Server class to make them live
 * objects and internally track the status (done or working) of the handler.
 * This way, the implementor of a Server handler doesn't need to worry about
 * any of this stuff.
 */
template <typename Functor>
class Handler {
   public:
    Handler(IO::Socket&& client, Functor& handler)
        : client(std::move(client)), handler(handler) {
        /* starts the internal thread */
        this->thread = std::thread(&Handler::handle_client, this);
    }
    ~Handler() {
        this->thread.join();
        std::cout << "killed" << std::endl;
    };

    /**
     * @brief Executes the custom handler and updates the "done" status.
     */
    void handle_client() {
        try {
            this->handler(this->client);
        } catch (...) {
            this->done = true;
            throw;
        }
        this->done = true;
    }

    /**
     * @brief Whether the handler finished it's execution.
     *
     * @return Execution status.
     */
    bool is_done() {
        return this->done;
    }

   private:
    /** Client socket. */
    IO::Socket client;
    /** Custom handler. */
    Functor& handler;
    /** Status flag. */
    std::atomic<bool> done{false};
    /** Internal thread to execute the custom handler. */
    std::thread thread;
};

/**
 * @brief Server that accepts multiple clients in their own threads.
 */
template <typename Functor>
class Server {
   public:
    Server(const std::string& port) {
        /* sets the internal socket in passive mode */
        this->socket.bind(port);
        this->socket.listen();
        this->exit_thread = std::thread(&Server::Server::exit_handler, this);
    }

    ~Server() {
        for (auto* handler : this->handlers) {
            delete handler;
        }
        this->exit_thread.join();
    }

    /** API */

    /**
     * @brief Blocks until a client connects to the server. Once a client is
     * received, executes the given `handler` passing it the new client's
     * socket.
     *
     * @param handler A functor that only receives a socket reference.
     */
    void handle_client(Functor& handler) {
        /* blocks until a client connects to the server */
        IO::Socket client = this->socket.accept();

        /* removes handlers that already finished */
        this->handlers_cleanup();

        /* allocates in heap because the live object requires a fixed memory
         * address */
        Handler<Functor>* h = new Handler<Functor>(std::move(client), handler);
        this->handlers.push_back(h);
    }

   private:
    /** Internal server socket. */
    IO::Socket socket;
    /** Internal array of client handlers */
    std::vector<Handler<Functor>*> handlers;
    /** Thread that waits for the exit signal. */
    std::thread exit_thread;

    /**
     * @brief Removes handlers that have already finished executing.
     */
    void handlers_cleanup() {
        /* it is inefficient to remove elements from a vector, but this way the
         * code is simpler */
        auto i = std::begin(this->handlers);
        while (i != std::end(this->handlers)) {
            if ((*i)->is_done()) {
                delete *i;
                i = this->handlers.erase(i);
            } else {
                ++i;
            }
        }
    }

    /**
     * @brief Handles the "Exit" condition of the server.
     */
    void exit_handler() {
        char c = 0;
        while (c != 'q') {
            std::cin >> c;
        }

        /* if the loop finished, it means the exit signal was received */
        this->socket.shutdown();
    }
};
}  // namespace Server

#endif
