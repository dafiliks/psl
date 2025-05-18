#include <iostream>
#include "lexer.hpp"

int main(int argc, char** argv)
{
    Lexer lexer{argc, argv};
    lexer.lex();
    
    return 0;
}
