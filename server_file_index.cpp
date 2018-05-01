#include "server_file_index.h"
#include <condition_variable>
#include <string>

Server::FileIndex::FileIndex() {
}

Server::FileIndex::~FileIndex() {
}

/**
 * @brief Checks if a given hash is already added to the inedx.
 *
 * @param hash Hash to check.
 * @return false if the hash is not added.
 */
bool Server::FileIndex::exists(const std::string& hash) {
    /* checks if the hash exists */
    if (this->files.find(hash) == this->files.end()) {
        return false;
    }
    return true;
}

/**
 * @brief Inserts a new file/has into the index.
 *
 * @param name File name.
 * @param hash Hash.
 */
void Server::FileIndex::insert_file(const std::string& name,
                                    const std::string& hash) {
    if (this->exists(hash)) {
        throw Error::Exists{hash};
    }

    this->hashes[name].insert(hash);
    this->files[hash] = name;
}

/**
 * @brief Removes a file/hash from the index.
 *
 * @param name File name.
 * @param hash Hash.
 */
void Server::FileIndex::remove_file(const std::string& name,
                                    const std::string& hash) {
    if (this->exists(hash)) {
        this->hashes[name].erase(hash);
    }
    if (this->files.find(hash) != this->files.end()) {
        this->files.erase(hash);
    }
}

/**
 * @brief Gets the name of the file that corresponds the given hash.
 *
 * @param hash Hash to obtain the file name.
 * @return File name.
 */
const std::string& Server::FileIndex::get_file_name(
    const std::string& hash) const {
    return this->files.at(hash);
}

/**
 * @brief Returns an iterator to the files and associated hashes.
 *
 * @return Iterator to the set of hashes for each file.
 */
Server::FileIndex::const_iterator Server::FileIndex::begin() const {
    return this->hashes.begin();
}

/**
 * @brief Last iterator.
 *
 * @return Last iterator.
 */
Server::FileIndex::const_iterator Server::FileIndex::end() const {
    return this->hashes.end();
}
