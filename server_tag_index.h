#ifndef SERVER_TAG_INDEX_H_
#define SERVER_TAG_INDEX_H_

#include <map>
#include <set>
#include <string>
#include "common_error.h"

namespace Server {
class TagIndex {
   public:
    TagIndex();
    ~TagIndex();

    void swap(TagIndex& l, TagIndex& r);

    const std::set<std::string>& get_hashes(const std::string& tag) const;
    void add(const std::string& tag, const std::set<std::string>& hashes);

   private:
    std::map<std::string, std::set<std::string>> hashes;
};
}  // namespace Server

#endif
