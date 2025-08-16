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
	/* Store appropriate error message */
	m_error << et_to_string(m_type) << " error: " <<  message << "\n";
}

Error::Error(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source,
             const ErrorType type) : m_source(std::move(source)), m_type(type) /* Initialize members */
{
	/* Store appropriate error message */
	m_error << et_to_string(m_type) << " error: " << row << ":" << col << ": " << message << "\n";

	/* Add the part of the source code which caused the error */
	m_error << add_source_error(row, col).str();
}

[[nodiscard]] const std::string& Error::get_source() const
{
	return m_source; /* Return the source contents */
}

[[nodiscard]] const ErrorType& Error::get_type() const
{
	return m_type; /* Return error type */
}

[[nodiscard]] std::string Error::what() const
{
	return m_error.str(); /* Return final error message */
}

std::ostringstream Error::add_source_error(const std::size_t row, const std::size_t col) const
{
	std::ostringstream m_source_error{}; /* Holds the source error */

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
	m_source_error << row << " | ";

	/* Print the entire line containing the error to standard error output */
	m_source_error << m_source.substr(start, end - start) << "\n";

	/* Return the source error stream */
	return m_source_error;
}
