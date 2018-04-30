#ifndef SERVER_FILE_INDEX_H_
#define SERVER_FILE_INDEX_H_

#include <condition_variable>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include "common_error.h"

namespace Server {
class FileIndex {
   public:
    using const_iterator =
        std::map<std::string, std::set<std::string>>::const_iterator;

    FileIndex();
    ~FileIndex();

    /** api */
    void insert_file(const std::string& name, const std::string& hash);
    void remove_file(const std::string& name, const std::string& hash);

    /** query */
    bool exists(const std::string& hash);
    const std::string& get_file_name(const std::string& hash) const;

    /** iterators */
    const_iterator begin() const;
    const_iterator end() const;

   private:
    /* maps the file name to a group of hashes */
    std::map<std::string, std::set<std::string>> hashes;
    /* maps the hash to a file */
    std::map<std::string, std::string> files;
};
}  // namespace Server

#endif
