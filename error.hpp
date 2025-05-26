#ifndef ERROR_HPP
#define ERROR_HPP

#include <iostream>

static void show_source_error(std::string source,
                       std::size_t line,
                       std::size_t col)
{
    std::size_t line_char_count{std::to_string(line).size()};
    for (std::size_t i = line_char_count; i < 5; i++) {
        std::cerr << " ";
    }
    std::cerr << line << " | ";
    // NOTE: using start/end indexes to take a substring of the source
    std::size_t start{}, end{};
    for (std::size_t i = 0; i < line - 1; i++) {
        start = source.find("\n", start);
        start++;
    }
    end = source.find('\n', start);
    // NOTE: if there isn't a corresponding, \n, look for \0
    // HINT: might happen on the last line of the source
    if (end == std::string::npos) {
        end = source.find('\0', start);
    }
    std::cerr << source.substr(start, end - start) << "\n";
    // NOTE: alligns differently if line number is > 5 characters
    for (std::size_t i = 5; i < line_char_count; i++) {
        std::cout << " ";
    }
    std::cerr << "      | ";
    // NOTE: alligns the arrow so it points to the incorrect part of source
    for (std::size_t i = 0; i < col - 1; i++) {
        std::cerr << " ";
    }
    std::cerr << "^\n";
}

static void error(std::string message)
{
    std::cerr << "error: " << message << "\n";
    exit(EXIT_FAILURE);
}

static void error_lc(std::string source,
              std::string file_name,
              std::string message,
              std::size_t line,
              std::size_t col)
{
    std::cerr << file_name << ":"         << line
                           << ":"         << col
                           << ": error: " << message << "\n";
    show_source_error(source, line, col);
    exit(EXIT_FAILURE);
}

#endif
