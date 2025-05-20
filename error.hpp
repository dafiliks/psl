#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include <sstream>

#define error(message)                               \
    std::stringstream ss{};                          \
    ss << message;                                   \
    std::cerr << "PsL: ERROR: " << ss.str() << "\n"; \
    exit(EXIT_FAILURE);                              \

#endif
