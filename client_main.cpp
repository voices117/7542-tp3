#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include "client_versioner.h"
#include "common_comm_socket.h"
#include "common_error.h"

int main(int argc, const char* argv[]) {
    if (argc < 4) {
        std::cout << "parametros invalidos" << std::endl;
        return 0;
    }

    const char* ip = argv[1];
    const char* service = argv[2];
    std::string action{argv[3]};

    try {
        IO::CommSocket comm{ip, service};
        Client::Versioner v{comm};

        /* selects the appropriate action from the command line arguments */
        if (action == "push" && argc == 6) {
            v.push(argv[4], argv[5]);
        } else if (action == "pull" && argc == 5) {
            v.pull(argv[4]);
        } else if (action == "tag" && argc > 5) {
            std::vector<std::string> hashes;
            for (int i = 5; i < argc; i++) {
                hashes.push_back(argv[i]);
            }
            v.tag(argv[4], hashes);
        } else {
            throw Error::Error{"parametros invalidos"};
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "Error desconocido" << std::endl;
    }

    return EXIT_SUCCESS;
}
