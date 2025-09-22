/* utils/error_types.cpp by David Filiks */
/* The error types implementation for the PsL compiler */

#include "error_types.hpp"

CLIArgsError::CLIArgsError(const std::string_view message)
/* Initialize Error object */
: Error(message, ErrorType::CLI_ARGS) {}

LexError::LexError(const std::string_view message)
/* Initialize Error object */
: Error(message, ErrorType::LEX) {}

LexError::LexError(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source)
/* Initialize Error object using more data */
: Error(message, row, col, source, ErrorType::LEX) {}

ParseError::ParseError(const std::string_view message)
/* Initialize Error object */
: Error(message, ErrorType::PARSE) {}

ParseError::ParseError(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source)
/* Initialize Error object using more data */
: Error(message, row, col, source, ErrorType::PARSE) {}

StackError::StackError(const std::string_view message)
/* Initialize Error object */
: Error(message, ErrorType::STACK) {}

GenError::GenError(const std::string_view message)
/* Initialize Error object */
: Error(message, ErrorType::GEN) {}

GenError::GenError(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source)
/* Initialize Error object using more data */
: Error(message, row, col, source, ErrorType::GEN) {}

CompileError::CompileError(const std::string_view message)
/* Initialize Error object */
: Error(message, ErrorType::COMPILE) {}

CompileError::CompileError(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source)
/* Initialize Error object using more data */
: Error(message, row, col, source, ErrorType::COMPILE) {}
