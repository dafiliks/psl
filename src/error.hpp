#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>

struct Error {
    Error(std::string source, std::string m_file_name);
    Error() = default;
    ~Error() = default;

    void show_source_error(std::size_t line, std::size_t col);
    [[noreturn]] void error(std::string message);
    [[noreturn]] void error_lc(std::string message, std::size_t line, std::size_t col);

    std::string m_source{};
    std::string m_file_name{};
};

#endif
