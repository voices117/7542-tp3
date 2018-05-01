#include <iostream>
#include "server.h"
#include "server_versioner.h"

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        std::cout << "parametros invalidos" << std::endl;
        return 0;
    }

    try {
        Server::Versioner versioner{argv[2]};
        Server::Server<Server::Versioner> server{argv[1]};

        /* runs until accept is interrupted */
        while (true) {
            server.handle_client(versioner);
        }
    } catch (const IO::Interrupted& e) {
        /* this exception means that the server must quit */
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "Error desconocido" << std::endl;
    }
}
