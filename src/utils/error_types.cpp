/* utils/error_types.cpp by David Filiks */
/* The error types implementation for the PsL compiler */

#include "error_types.hpp"
#include "error.hpp"

/* CLI Args errors */

CLIArgsError::CLIArgsError(const std::string_view message) : Error(message, ErrorType::CLI_ARGS) /* Initializes Error object */ {}

/* Lexing errors */

LexError::LexError(const std::string_view message) : Error(message, ErrorType::LEX) /* Initializes Error object */ {}

LexError::LexError(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source)
: Error(message, row, col, source, ErrorType::LEX) /* Initializes Error object */ {}

/* Parsing errors */

ParseError::ParseError(const std::string_view message) : Error(message, ErrorType::PARSE) /* Initializes Error object */ {}

ParseError::ParseError(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source)
: Error(message, row, col, source, ErrorType::PARSE) /* Initializes Error object */ {}

/* Stack errors */

StackError::StackError(const std::string_view message) : Error(message, ErrorType::STACK) /* Initializes Error object */ {}

/* Generator errors */

GenError::GenError(const std::string_view message) : Error(message, ErrorType::GEN) /* Initializes Error object */ {}

GenError::GenError(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source)
: Error(message, row, col, source, ErrorType::GEN) /* Initializes Error object */ {}

/* Compilation errors */

CompileError::CompileError(const std::string_view message) : Error(message, ErrorType::COMPILE) /* Initializes Error object */ {}

CompileError::CompileError(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source)
: Error(message, row, col, source, ErrorType::COMPILE) /* Initializes Error object */ {}