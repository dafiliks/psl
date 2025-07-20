#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <string>

#include "../frontend/lexer.hpp"
#include "../frontend/parser.hpp"
#include "../backend/gen.hpp"
#include "../utils/cmdargs.hpp"

class Compiler {
public:
	Compiler(CmdArgs args);

	void compile();

private:
	void compile_to_cpp();
	void compile_to_output_target(const std::string_view& cpp_file);

	// private members
	CmdArgs m_args;
	Lexer m_lexer;
	Parser m_parser;
	Generator m_gen;
};

#endif
