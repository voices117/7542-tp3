#ifndef COMMON_ERROR_H_
#define COMMON_ERROR_H_

#include <exception>
#include <string>

namespace Error {
class Error : public std::exception {
   public:
    Error(const char* fmt, ...);
    virtual ~Error();

    virtual const char* what() const noexcept;

   private:
    std::string message;
};

class Exists : public Error {
   public:
    Exists(const std::string& resource)
        : Error("%s already exists", resource.c_str()) {
    }
    ~Exists() {
    }
};

class NotFound : public Error {
   public:
    NotFound(const std::string& resource)
        : Error("%s not found", resource.c_str()) {
    }
    ~NotFound() {
    }
};
}  // namespace Error

#endif
