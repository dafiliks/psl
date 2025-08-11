/* utils/error_types.hpp by David Filiks */
/* The error types header for the PsL compiler */

#ifndef ERROR_TYPES_HPP
#define ERROR_TYPES_HPP

#include "error.hpp"

/* CLIArgsError struct which inherits from Error and is responsible for outputting CLI argument errors */
struct CLIArgsError : public Error
{
	/* Constructs a CLIArgsError object */
	/* Param: const std::string_view - the message to be displayed upon cli args error */
	CLIArgsError(const std::string_view message);
};

/* LexError struct which inherits from Error and is responsible for outputting lexing errors */
struct LexError : public Error
{
	/* Constructs a LexError object */
	/* Param: const std::string_view - the message to be displayed upon lex error */
	LexError(const std::string_view message);

	/* Constructs a LexError object with more data */
	/* Param: const std::string_view - the message to be displayed upon lex error */
	/* Param: const std::size_t - the row that the lex error occurs on */
	/* Param: const std::size_t - the col that the lex error occurs on */
	/* Param: const std::string_view - the source code in which the lex error occurs */
	LexError(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source);
};

/* ParseError struct which inherits from Error and is responsible for outputting parsing errors */
struct ParseError : public Error
{
	/* Constructs a ParseError object */
	/* Param: const std::string_view - the message to be displayed upon parse error */
	ParseError(const std::string_view message);

	/* Constructs a ParseError object with more data */
	/* Param: const std::string_view - the message to be displayed upon parse error */
	/* Param: const std::size_t - the row that the parse error occurs on */
	/* Param: const std::size_t - the col that the parse error occurs on */
	/* Param: const std::string_view - the source code in which the parse error occurs */
	ParseError(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source);
};

/* StackError struct which inherits from Error and is responsible for outputting stack errors */
struct StackError : public Error
{
	/* Constructs a StackError object */
	/* Param: const std::string_view - the message to be displayed upon stack error */
	StackError(const std::string_view message);
};

/* GenError struct which inherits from Error and is responsible for outputting code generation errors */
struct GenError : public Error
{
	/* Constructs a GenError object */
	/* Param: const std::string_view - the message to be displayed upon gen error */
	GenError(const std::string_view message);

	/* Constructs a GenError object with more data */
	/* Param: const std::string_view - the message to be displayed upon gen error */
	/* Param: const std::size_t - the row that the gen error occurs on */
	/* Param: const std::size_t - the col that the gen error occurs on */
	/* Param: const std::string_view - the source code in which the gen error occurs */
	GenError(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source);
};

/* CompileError struct which inherits from Error and is responsible for outputting compilation errors */
struct CompileError : public Error
{
	/* Constructs a CompileError object */
	/* Param: const std::string_view - the message to be displayed upon compile error */
	CompileError(const std::string_view message);

	/* Constructs a CompileError object with more data */
	/* Param: const std::string_view - the message to be displayed upon compile error */
	/* Param: const std::size_t - the row that the compile error occurs on */
	/* Param: const std::size_t - the col that the compile error occurs on */
	/* Param: const std::string_view - the source code in which the compile error occurs */
	CompileError(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source);
};

#endif