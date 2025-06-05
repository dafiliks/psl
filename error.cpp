#include <string>
#include <iostream>
#include "error.hpp"

Error::Error(std::string source, std::string file_name)
: m_source(source), m_file_name(file_name) {}

// TODO: convoluted function, refactor later
void Error::show_source_error(std::size_t line, std::size_t col)
{
	std::size_t line_char_count{std::to_string(line).size()};
	for (std::size_t i = line_char_count; i < 5; i++) {
		std::cerr << " ";
	}
	std::cerr << line << " | ";

	std::size_t start{}, end{};
	for (std::size_t i = 0; i < line - 1; i++) {
		start = m_source.find("\n", start);
		start++;
	}
	end = m_source.find('\n', start);

	if (end == std::string::npos) {
		end = m_source.find('\0', start);
	}
	std::cerr << m_source.substr(start, end - start) << "\n";

	for (std::size_t i = 5; i < line_char_count; i++) {
		std::cerr << " ";
	}
	std::cerr << "      | ";

	for (std::size_t i = 0; i < col - 1; i++) {
		std::cerr << " ";
	}
	std::cerr << "^\n";
}

[[noreturn]] void Error::error(std::string message)
{
	std::cerr << "error: " << message << "\n";
	exit(EXIT_FAILURE);
}

[[noreturn]] void Error::error_lc(std::string message, std::size_t line, std::size_t col)
{
	std::cerr << m_file_name << ":"         << line
	                       << ":"         << col
	                       << ": error: " << message << "\n";
	show_source_error(line, col);
	exit(EXIT_FAILURE);
}
