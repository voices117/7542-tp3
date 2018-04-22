#include "common_error.h"
#include <cstdarg>
#include <cstdio>

/**
 * @brief Constructor. Takes a printf-like format string and a corresponding set
 * of arguments.
 *
 * @param fmt printf-like format string.
 * @param ... Arguments that correspond to `fmt`.
 */
TP3::Error::Error(const char* fmt, ...) {
    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);

    try {
        std::size_t size = std::vsnprintf(nullptr, 0, fmt, args) + 1;

        this->message.reserve(size);
        std::vsnprintf(&this->message.front(), size, fmt, args_copy);

        va_end(args_copy);
        va_end(args);
    } catch (...) {
        va_end(args_copy);
        va_end(args);
        throw;
    }
}

TP3::Error::~Error() {
}

/**
 * @brief Shows the error message contained in the exception object.
 *
 * @return Exception message.
 */
const char* TP3::Error::what() const noexcept {
    return this->message.c_str();
}
