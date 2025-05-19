#include <vector>
#include <cctype>
#include <fstream>
#include <cstring>
#include <iostream>
#include <cassert>
#include "lexer.hpp"

Lexer::Lexer(int argc, char** argv)
{
    assert(argc != 1);
    if (argv[1][strlen(argv[1]) - 6] != 'p' ||
        argv[1][strlen(argv[1]) - 5] != 's' ||
        argv[1][strlen(argv[1]) - 4] != 'e' ||
        argv[1][strlen(argv[1]) - 3] != 'u' ||
        argv[1][strlen(argv[1]) - 2] != 'd' ||
        argv[1][strlen(argv[1]) - 1] != 'o') {
        std::cout << "PsL: ERROR: File lacks \".pseudo\" extension\n";
        exit(EXIT_FAILURE);
    }

    std::ifstream file{};
    file.open(argv[1]);

    if (!file.is_open()) {
        std::cout << "PsL: ERROR: File \"" << argv[1] << "\" could not be opened\n";
        exit(EXIT_FAILURE);
    }

    std::string source{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    m_source = source + '\0';

    file.close();
}

std::vector<Token> Lexer::lex()
{
    while (peek() != '\0') {
        if (isalpha(peek())) {
            do {
                m_buffer += eat();
            } while (!is_separator(peek()) && isalnum(peek()));
            m_tokens.push_back({Token_Type::IDENTIFIER, m_buffer});
            m_buffer.clear();
        } else if (isdigit(peek())) {
            Token_Type type{Token_Type::INT_LIT};
            do {
                m_buffer += eat();
                if (peek() == '.') {
                    type = Token_Type::FLOAT;
                } else if (!isdigit(peek())) break;
            } while (!is_separator(peek()));
            m_tokens.push_back({type, m_buffer});
            m_buffer.clear();
        } else {
            eat();
        }
    }

    m_tokens.push_back({Token_Type::END_OF_FILE, m_buffer});
    return m_tokens;
}

char Lexer::peek(std::size_t dist)
{
    assert(m_index + dist <= m_source.size());
    return m_source.at(m_index + dist);
}

char Lexer::eat(std::size_t dist)
{
    assert(m_index + dist <= m_source.size());
    m_index += dist;
    return m_source.at(m_index - dist);
}

bool Lexer::is_separator(char chr) const
{
    return chr == ' ' || chr == '\n' || chr == '\t' || chr == '\0';
}
