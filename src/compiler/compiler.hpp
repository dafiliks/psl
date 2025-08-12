/* compiler/compiler.hpp by David Filiks */
/* The compiler interface header for the PsL compiler */

#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <string>

#include "../utils/cliargs.hpp"
#include "../frontend/parser.hpp"
#include "../compiler/compilation_targets.hpp"
#include "../frontend/lexer.hpp"
#include "../backend/gen.hpp"
#include "../utils/error_types.hpp"

/* Compiler class, responsible for executing all compilation stages and outputting target ouput */
class Compiler
{

/* Public members */
public:

	/* Functions */

	/* Construct a Compiler object */
	/* Param: const CLIArgs& - the CLI arguments passed to the program */
	Compiler(const CLIArgs& args);

	/* Carry out all compilation stages */
	void compile();

/* Private members */
private:

	/* Functions */

	/* Compiles source file to a C++20 compliant ".cpp" file */
	void compile_to_cpp();

	/* Compiles a ".cpp" file to the output target as specified by CLI argument flags */
	/* Param: const std::string_view - the path to the cpp file */
	void compile_to_output_target(const std::string_view cpp_file);

	/* Variables */

	CLIArgs m_args; /* The CLI arguments passed to the program */
	Lexer m_lexer; /* The primary lexer object */
	Parser m_parser; /* The primary parser object */
	Generator m_gen; /* The primary generator object */
};

#endif
