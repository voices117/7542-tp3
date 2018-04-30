#include "server_versioner.h"
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <vector>

Server::Versioner::Versioner() {
}

Server::Versioner::Versioner(const std::string& file_name)
    : index_file_name(file_name) {
    std::ifstream file{file_name};

    /* reads the files */
    while (file) {
        std::string type, name, hash;

        file >> type >> name;

        if (!file) {
            break;
        }
        if (type == "f") {
            while (file && (file >> hash, hash != ";")) {
                this->file_index.insert_file(name, hash);
            }
        } else if (type == "t") {
            std::set<std::string> hashes;
            while (file && (file >> hash, hash != ";")) {
                hashes.insert(hash);
            }
            this->tag_index.add(name, hashes);
        } else {
            throw TP3::Error{"Unexpected type %s", type.c_str()};
        }
    }
}

Server::Versioner::~Versioner() {
    std::ofstream file{this->index_file_name};
    this->save(file);
}

/**
 * @brief Push handler.
 *
 * @param comm Communication endpoint.
 */
void Server::Versioner::push(IO::Comm& comm) {
    std::string filename, hash;

    /* reads the cmd data */
    comm >> filename >> hash;

    /* we only needs to check for readers because writers lock the mutex until
     * done */
    std::unique_lock<std::mutex> lock(this->mutex);

    try {
        this->file_index.insert_file(filename, hash);

        /* sends the response */
        comm << IO::Response::OK;

        /* reads the file */
        std::ofstream file{hash, std::ios::binary};
        comm >> file;
    } catch (const TP3::Exists& e) {
        comm << IO::Response::Error;
        this->file_index.remove_file(filename, hash);
    }
}

/**
 * @brief Pull handler.
 *
 * @param comm Communication endpoint.
 */
void Server::Versioner::pull(IO::Comm& comm) {
    try {
        std::unique_lock<std::mutex> lock(this->mutex);

        /* reads the tag name */
        std::string tag;
        comm >> tag;

        /* gets the associated hashes */
        const auto& hashes = this->tag_index.get_hashes(tag);

        comm << IO::Response::OK;
        comm << hashes.size();

        for (const std::string& hash : hashes) {
            comm << this->file_index.get_file_name(hash);

            /* sends the file content */
            std::ifstream file{hash, std::ios::binary};
            comm << file;
        }
    } catch (const TP3::NotFound& e) {
        comm << IO::Response::Error;
    }
}

/**
 * @brief Tag handler.
 *
 * @param comm Communication endpoint.
 */
void Server::Versioner::tag(IO::Comm& comm) {
    std::string name;
    uint32_t num_hashes;
    std::set<std::string> hashes;

    try {
        std::unique_lock<std::mutex> lock(this->mutex);

        comm >> num_hashes >> name;
        for (uint32_t i = 0; i < num_hashes; i++) {
            std::string hash;
            comm >> hash;
            hashes.insert(hash);
        }

        /* checks that all the hashes exist */
        for (const auto& hash : hashes) {
            if (!this->file_index.exists(hash)) {
                comm << IO::Response::Error;
                return;
            }
        }

        this->tag_index.add(name, hashes);
        comm << IO::Response::OK;
    } catch (const std::exception& e) {
        comm << IO::Response::Error;
    }
}

/**
 * @brief Server request handler.
 *
 * @param client The newly connected client.
 */
void Server::Versioner::operator()(TP3::Socket&& client) {
    try {
        IO::CommSocket comm{std::move(client)};
        uint8_t cmd_id;
        comm >> cmd_id;

        switch (cmd_id) {
            case 1:
                this->push(comm);
                break;
            case 2:
                this->tag(comm);
                break;
            case 3:
                this->pull(comm);
                break;
            default:
                std::cerr << "Invalid ID " << cmd_id << std::endl;
                break;
        }
    } catch (const IO::CommError& e) {
        /* if the client closed the connection nothing is done */
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Saves the index in a file.
 *
 * @param file File to save the index to.
 */
void Server::Versioner::save(std::ofstream& file) {
    /* writes the files */
    for (auto& pair : this->file_index) {
        file << "f " << pair.first << " ";
        for (const std::string& hash : pair.second) {
            file << hash << " ";
        }
        file << ";" << std::endl;
    }

    /* writes the tags */
    for (auto& pair : this->tag_index) {
        file << "t " << pair.first << " ";
        for (const std::string& hash : pair.second) {
            file << hash << " ";
        }
        file << ";" << std::endl;
    }
}
