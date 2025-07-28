#include <string>
#include <iostream>
#include <print>
#include <filesystem>
#include <cstdlib>

#include "compiler.hpp"
#include "../utils/cmdargs.hpp"
#include "../frontend/parser.hpp"
#include "../frontend/lexer.hpp"
#include "../backend/gen.hpp"

Compiler::Compiler(CmdArgs args) : m_args(args) {}

void Compiler::compile()
{
	compile_to_cpp();
	compile_to_output_target(m_args.get_source_path());
}

void Compiler::compile_to_cpp()
{
	m_lexer = Lexer{m_args.get_source_code()};
	m_lexer.lex();

	m_parser = Parser{m_lexer};
	m_parser.parse();

	m_gen = Generator{m_parser};
	m_gen.gen(std::filesystem::path{m_args.get_source_path()}.replace_extension(std::filesystem::path{".cpp"}));
}

void Compiler::compile_to_output_target(const std::string_view &cpp_file)
{
	if (m_args.get_target_output_flag() == "-exe")
	{
		std::string output_file{std::string{cpp_file}.substr(0, std::string{cpp_file}.find("."))};
		std::string command_str{"g++ " + output_file + ".cpp -o " + output_file + " && ./" + output_file};

		std::system(command_str.c_str());
	}
}
