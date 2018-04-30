#ifndef SERVER_TAG_INDEX_H_
#define SERVER_TAG_INDEX_H_

#include <map>
#include <set>
#include <string>
#include "common_error.h"

namespace Server {
class TagIndex {
   public:
    using const_iterator =
        std::map<std::string, std::set<std::string>>::const_iterator;

    TagIndex();
    ~TagIndex();

    /** api */
    const std::set<std::string>& get_hashes(const std::string& tag) const;
    void add(const std::string& tag, const std::set<std::string>& hashes);

    /** iterators */
    const_iterator begin() const;
    const_iterator end() const;

   private:
    std::map<std::string, std::set<std::string>> hashes;
};
}  // namespace Server

#endif
