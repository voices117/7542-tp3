#include "server.h"

Server::Server::Server(const std::string& port) {
    /* sets the internal socket in passive mode */
    this->socket.bind(port);
    this->socket.listen();
}

Server::Server::~Server() {
    // TODO: force close
}
