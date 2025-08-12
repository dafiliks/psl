/* utils/error.cpp by David Filiks */
/* The error implementation for the PsL compiler */

#include "error.hpp"

std::string_view et_to_string(const ErrorType type)
{
	/* Output corresponding string from ErrorType */

	switch (type)
	{
		/* Typical error types */

		case (ErrorType::CLI_ARGS): return "cli args";
		case (ErrorType::LEX):      return "lex";
		case (ErrorType::PARSE):    return "parse";
		case (ErrorType::STACK):    return "stack";
		case (ErrorType::GEN):      return "gen";
		case (ErrorType::COMPILE):  return "compile";

		/* Unknown error type */

		default:                    return "unknown";
	}
}

Error::Error(const std::string_view message, const ErrorType type)
: m_type(type) /* Initialize error type */
{
	/* Display appropriate error message */
	std::cerr << et_to_string(m_type) << " error: " <<  message << "\n";

	/* Terminate program execution with EXIT_FAILURE */
	std::exit(EXIT_FAILURE);
}

Error::Error(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source,
             const ErrorType type) : m_source(std::move(source)), m_type(type) /* Initialize members */
{
	/* Display appropriate error message */
	std::cerr << et_to_string(m_type) << " error: " << row << ":" << col << ": " << message << "\n";

	/* Show the part of the source code which caused the error */
	show_source_error(row, col);

	/* Terminate program execution with EXIT_FAILURE */
	std::exit(EXIT_FAILURE);
}

[[nodiscard]] const std::string& Error::get_source() const
{
	return m_source; /* Return the source contents */
}

[[nodiscard]] const ErrorType& Error::get_type() const
{
	return m_type; /* Return error type */
}

void Error::show_source_error(const std::size_t row, const std::size_t col) const
{
	std::size_t start{}; /* Source error start index */
	std::size_t end{}; /* Source error end index */

	/* Find the start index of the error */
	for (std::size_t i{}; i < row - 1; i++)
	{
		/* Move to next newline */
		start = m_source.find("\n", start);

		/* Consume the newline character */
		start++;
	}

	/* Find the end index of the error */
	end = m_source.find('\n', start);

	/* If no newline found */
	if (end == std::string::npos)
	{
		/* Search for end of file position */
		end = m_source.find('\0', start);
	}

	/* Print the row number next to source */
	std::cerr << row << " | ";

	/* Print the entire line containing the error to standard error output */
	std::cerr << m_source.substr(start, end - start) << "\n";
}
