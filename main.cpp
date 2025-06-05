#include <iostream>
#include "lexer.hpp"
#include "parser.hpp"
#include "gen.hpp"

int main(int argc, char** argv)
{
	Lexer lexer{argc, argv};
	lexer.lex();

	Parser parser{lexer};
    parser.parse();

	Generator gen{std::move(parser)}; // fucking unique ptrs
	gen.gen();

	return 0;
}
