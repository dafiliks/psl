#include <string>
#include <print>
#include <iostream>

#include "error.hpp"

Error::Error(const std::string_view& message)
{
	std::print("error: {}", message);
	std::exit(EXIT_FAILURE);
}

Error::Error(const std::string_view& message, const std::size_t line, const std::size_t col, const std::string_view& source)
{
	std::print("error: {}:{}: {}", line, col, message);
	show_source_error(line, col);
	std::exit(EXIT_FAILURE);
}

void Error::show_source_error(const std::size_t line, const std::size_t col)
{
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
}
