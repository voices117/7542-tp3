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

class Exists : public TP3::Error {
   public:
    Exists(const std::string& resource)
        : Error("%s already exists", resource.c_str()) {
    }
    ~Exists() {
    }
};

class NotFound : public TP3::Error {
   public:
    NotFound(const std::string& resource)
        : Error("%s not found", resource.c_str()) {
    }
    ~NotFound() {
    }
};
}  // namespace TP3

#endif
