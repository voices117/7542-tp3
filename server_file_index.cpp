#include "server_file_index.h"
#include <condition_variable>

TP3::FileIndex::FileIndex() {
}

TP3::FileIndex::~FileIndex() {
}

bool TP3::FileIndex::exists(const std::string& hash) {
    /* checks if the hash exists */
    if (this->files.find(hash) == this->files.end()) {
        return false;
    }
    return true;
}

void TP3::FileIndex::insert_file(const std::string& name,
                                 const std::string& hash) {
    if (this->exists(hash)) {
        throw TP3::Exists{hash};
    }

    this->hashes[name].insert(hash);
    this->files[hash] = name;
}

const std::string& TP3::FileIndex::get_file_name(const std::string& hash) const {
    return this->files.at(hash);
}
