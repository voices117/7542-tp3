#include "server.h"
#include <iostream>

Server::Server::Server(const std::string& port) {
    /* sets the internal socket in passive mode */
    this->socket.bind(port);
    this->socket.listen();
    this->exit_thread = std::thread(&Server::Server::exit_handler, this);
}

Server::Server::~Server() {
    for (auto& handler : this->handlers) {
        handler.join();
    }
    this->exit_thread.join();
}

/**
 * @brief Waits for the "exit" signal.
 *
 */
void Server::Server::exit_handler() {
    char c = 0;
    while (c != 'q') {
        std::cin >> c;
    }

    /* if the loop finished, it means the exit signal was received */
    this->socket.shutdown();
}
