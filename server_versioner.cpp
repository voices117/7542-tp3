#include "server_versioner.h"
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "common_rw_lock.h"

Server::Versioner::Versioner() {
}

/**
 * @brief Initializes a Versioner object from an index in the given file name.
 *
 * @param file_name The name of the index file.
 */
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
            throw Error::Error{"Unexpected type %s", type.c_str()};
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

    Concurrency::WriteLock lock(this->lock);

    try {
        this->file_index.insert_file(filename, hash);

        /* sends the response */
        comm << IO::Response::OK;

        /* reads the file */
        std::ofstream file{hash, std::ios::binary};
        comm >> file;
    } catch (const Error::Exists& e) {
        comm << IO::Response::Error;
    } catch (...) {
        this->file_index.remove_file(filename, hash);
        throw;
    }
}

/**
 * @brief Pull handler.
 *
 * @param comm Communication endpoint.
 */
void Server::Versioner::pull(IO::Comm& comm) {
    try {
        /* pull only requires read access */
        Concurrency::ReadLock lock(this->lock);

        /* reads the tag name */
        std::string tag;
        comm >> tag;

        /* gets the associated hashes */
        const auto& hashes = this->tag_index.get_hashes(tag);

        comm << IO::Response::OK << static_cast<uint32_t>(hashes.size());

        for (const std::string& hash : hashes) {
            comm << this->file_index.get_file_name(hash);

            /* sends the file content */
            std::ifstream file{hash, std::ios::binary};
            comm << file;
        }
    } catch (const Error::NotFound& e) {
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
        Concurrency::WriteLock lock(this->lock);

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
void Server::Versioner::operator()(IO::Socket& client) {
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
