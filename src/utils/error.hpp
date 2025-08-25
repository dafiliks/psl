/* utils/error.hpp by David Filiks */
/* The error header for the PsL compiler */

#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include <iostream>
#include <sstream>

/* Enum class of all the possible error stages for the PsL compiler */
enum class ErrorType
{
	CLI_ARGS, /* Represents a cli args error */
	LEX, /* Represents a lexing error */
	PARSE, /* Represents a parsing error */
	STACK, /* Represents a stack error */
	GEN, /* Represents a generator error */
	COMPILE, /* Represents a compilation error */
};

/* Returns the corresponding string equivalent for a particular ErrorType */
/* Param: const ErrorType - the error type */
/* Returns: std::string_view - the string equivalent */
std::string_view et_to_string(const ErrorType type);

/* Error class, responsible for erroring out and terminating the program upon fault */
class Error
{
/* Public members */
public:

	/* Functions */

	/* Constructs an Error object */
	/* Param: const std::string_view - the message to be displayed upon error */
	/* Param: const ErrorType - the type of error */
	Error(const std::string_view message, const ErrorType type);

	/* Constructs an Error object using more data */
	/* Param: const std::string_view - the message to be displayed upon error */
	/* Param: const std::size_t - the row that the error occurs on */
	/* Param: const std::size_t - the col that the error occurs on */
	/* Param: const std::string& - the source code in which the error occurs */
	/* Param: const ErrorType - the type of error */
	Error(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source, const ErrorType type);

	/* Getter function for the source file contents */
	/* Returns: const std::string& - the source file contents */
	[[nodiscard]] const std::string& get_source() const;

	/* Getter function for the error type */
	/* Returns: const ErrorType& - the error type */
	[[nodiscard]] const ErrorType& get_type() const;

	/* Getter function for the error message */
	/* Returns: std::string - the error message */
	[[nodiscard]] std::string what() const;

/* Private members */
private:

	/* Functions */

	/* Returns the line in the source code that the error occurs at */
	/* Param: const std::size_t - the row that the error occurs on */
	/* Param: const std::size_t - the col that the error occurs on */
	/* Returns: std::ostringstream - the source error string stream */
	std::ostringstream add_source_error(const std::size_t row, const std::size_t col) const;

	/* Variables */

	std::string m_source{}; /* The program source for displaying the error */
	ErrorType m_type{}; /* The type of error that is to be displayed */

	std::ostringstream m_error{}; /* The final error message */
};

#endif
