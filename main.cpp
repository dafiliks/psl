#include <iostream>
#include "lexer.hpp"
#include "parser.hpp"

int main(int argc, char** argv)
{
    Lexer lexer{argc, argv};
    lexer.lex();

    Parser parser{lexer};
    parser.parse();

    return 0;
}
