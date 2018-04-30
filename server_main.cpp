#include <iostream>
#include "server.h"
#include "server_versioner.h"

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        std::cout << "parametros invalidos" << std::endl;
        return 0;
    }

    try {
        std::ifstream file{argv[2]};
        Server::Versioner versioner{file};
        Server::Server server{argv[1]};

        bool quit = false;
        while (!quit) {
            server.handle_client(versioner);
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "Error desconocido" << std::endl;
    }
}
