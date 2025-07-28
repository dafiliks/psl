#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>

class Error
{
public:
	Error() = default;
	Error(const std::string_view &message);
	Error(const std::string_view &message, const std::size_t line, const std::size_t col, const std::string_view &source);

private:
	void show_source_error(const std::size_t line, const std::size_t col);

	std::string m_source{};
};

#endif
