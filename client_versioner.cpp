#include "client_versioner.h"
#include <unistd.h>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

/**
 * @brief Construct a new Versioner.
 *
 * @param server_address Server address.
 * @param port Port or service.
 */
Client::Versioner::Versioner(IO::Comm& comm) : comm(comm) {
}

Client::Versioner::~Versioner() {
}

/**
 * @brief Does a push operation on a file.
 *
 * @param file_name Name of the file to push.
 * @param hash File hash.
 */
void Client::Versioner::push(const std::string& file_name,
                             const std::string& hash) {
    const uint8_t pull_cmd_id = 1;

    if (access(file_name.c_str(), F_OK) == -1) {
        throw Error::Error{"Error: archivo inexistente."};
    }

    /* writes the command ID */
    this->comm << pull_cmd_id << file_name << hash;

    /* gets the server response */
    IO::Response response = IO::Response::Error;
    this->comm >> response;

    switch (response) {
        case IO::Response::Error:
            /* the error means that the hash already exists, so it's a no op */
            return;
        case IO::Response::OK:
            break;
        default:
            throw Error::Error{"Push: codigo de retorno invalido"};
    }

    std::ifstream file{file_name, std::ios::binary};
    this->comm << file;
}

/**
 * @brief Does a pull operation on a tag.
 *
 * @param tag Name of the tag to pull.
 */
void Client::Versioner::pull(const std::string& tag) {
    const uint8_t pull_cmd_id = 3;

    /* writes the command ID */
    this->comm << pull_cmd_id << tag;

    /* gets the server response */
    IO::Response response = IO::Response::Error;
    this->comm >> response;

    switch (response) {
        case IO::Response::OK:
            /* continues the operation */
            break;
        case IO::Response::Error:
            throw Error::Error{"Error: tag/hash incorrecto."};
        default:
            throw Error::Error{"Pull: codigo de retorno invalido"};
    }

    /* gets the number of files in the tag */
    uint32_t num_files;
    this->comm >> num_files;

    for (uint32_t i = 0; i < num_files; i++) {
        std::string file_name;
        this->comm >> file_name;

        /* writes to disk */
        std::ofstream file{file_name + "." + tag, std::ios::binary};
        this->comm >> file;
    }
}

/**
 * @brief Does a tag operation.
 *
 * @param tag The tag name.
 * @param hashes The list of hashes to include in the tag.
 */
void Client::Versioner::tag(const std::string& tag,
                            std::vector<std::string>& hashes) {
    const uint8_t tag_cmd_id = 2;

    this->comm << tag_cmd_id << static_cast<uint32_t>(hashes.size()) << tag;
    for (std::size_t i = 0; i < hashes.size(); i++) {
        this->comm << hashes[i];
    }

    IO::Response response = IO::Response::Error;
    this->comm >> response;

    switch (response) {
        case IO::Response::OK:
            break;
        case IO::Response::Error:
            throw Error::Error{"Error: tag/hash incorrecto."};
        default:
            throw Error::Error{"Tag: codigo de retorno invalido"};
    }
}
