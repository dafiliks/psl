#include <iostream>
#include <cstring>
#include <cstdlib>
#include "lexer.hpp"
#include "parser.hpp"
#include "gen.hpp"

// incomplete for now
void convert_to_output_target(char** argv) {
    if (strcmp(argv[2], "-exe") == 0) {
        std::string output_file{std::string{argv[1]}.substr(0, std::string{argv[1]}.find("."))};
        std::string command_str{"g++ " + output_file + ".cpp -o " + output_file + " && ./" + output_file};

        system(command_str.c_str());
    }
}

int main(int argc, char** argv)
{
	Lexer lexer{argc, argv};
	lexer.lex();

	Parser parser{lexer};
    parser.parse();

	Generator gen{std::move(parser)};
	gen.gen();

    convert_to_output_target(argv);

	return 0;
}
