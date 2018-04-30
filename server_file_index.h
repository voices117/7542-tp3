#ifndef SERVER_FILE_INDEX_H_
#define SERVER_FILE_INDEX_H_

#include <condition_variable>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include "common_error.h"

namespace TP3 {
class FileIndex {
   public:
    FileIndex();
    ~FileIndex();

    void swap(TP3::FileIndex& other);

    bool exists(const std::string& hash);
    void insert_file(const std::string& name, const std::string& hash);
    const std::string& get_file_name(const std::string& hash) const;

   private:
    /* maps the file name to a group of hashes */
    std::map<std::string, std::set<std::string>> hashes;
    /* maps the hash to a file */
    std::map<std::string, std::string> files;
};
}  // namespace TP3

#endif
