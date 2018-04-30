#include "server_tag_index.h"

Server::TagIndex::TagIndex() {
}

Server::TagIndex::~TagIndex() {
}

const std::set<std::string>& Server::TagIndex::get_hashes(
    const std::string& tag) const {
    try {
        return this->hashes.at(tag);
    } catch (const std::out_of_range& e) {
        throw TP3::NotFound{tag};
    }
}

void Server::TagIndex::add(const std::string& tag,
                           const std::set<std::string>& hashes) {
    /* checks if the tag already exists */
    if (this->hashes.find(tag) != this->hashes.end()) {
        throw TP3::Exists{tag};
    }

    /* inserts the hashes into the tag */
    this->hashes[tag].insert(hashes.begin(), hashes.end());
}
