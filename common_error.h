#ifndef COMMON_ERROR_H_
#define COMMON_ERROR_H_

#include <exception>
#include <string>

namespace TP3 {
class Error : public std::exception {
   public:
    Error(const char* fmt, ...);
    virtual ~Error();

    virtual const char* what() const noexcept;

   private:
    std::string message;
};
}  // namespace TP3

#endif
