#include "server_tag_index.h"

Server::TagIndex::TagIndex() {
}

Server::TagIndex::~TagIndex() {
}

/**
 * @brief Implements the swap for this class.
 *
 * @param l Left operand.
 * @param r Right operand.
 */
void Server::TagIndex::swap(TagIndex& l, TagIndex& r) {
    std::swap(l.hashes, r.hashes);
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
        throw TP3::NotFound{tag};
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
        throw TP3::Exists{tag};
    }

    /* inserts the hashes into the tag */
    this->hashes[tag].insert(hashes.begin(), hashes.end());
}
