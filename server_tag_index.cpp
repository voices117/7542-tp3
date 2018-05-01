#include "server_tag_index.h"
#include <set>
#include <string>

Server::TagIndex::TagIndex() {
}

Server::TagIndex::~TagIndex() {
}

/**
 * @brief Gets the list of hashes associated to a given tag.
 *
 * @param tag Tag to query.
 * @return Set of associated hashes.
 */
const std::set<std::string>& Server::TagIndex::get_hashes(
    const std::string& tag) const {
    try {
        return this->hashes.at(tag);
    } catch (const std::out_of_range& e) {
        throw Error::NotFound{tag};
    }
}

/**
 * @brief Adds a new tag into the index.
 *
 * @param tag Name of the new tag.
 * @param hashes List of associated hashes.
 */
void Server::TagIndex::add(const std::string& tag,
                           const std::set<std::string>& hashes) {
    /* checks if the tag already exists */
    if (this->hashes.find(tag) != this->hashes.end()) {
        throw Error::Exists{tag};
    }

    /* inserts the hashes into the tag */
    this->hashes[tag].insert(hashes.begin(), hashes.end());
}

/**
 * @brief Returns an iterator to the tags and associated hashes.
 *
 * @return Iterator to the set of hashes for each tag.
 */
Server::TagIndex::const_iterator Server::TagIndex::begin() const {
    return this->hashes.begin();
}

/**
 * @brief Last iterator.
 *
 * @return Last iterator.
 */
Server::TagIndex::const_iterator Server::TagIndex::end() const {
    return this->hashes.end();
}
