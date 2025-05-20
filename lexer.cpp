#include <vector>
#include <cctype>
#include <fstream>
#include <cstring>
#include <iostream>
#include <cassert>
#include "lexer.hpp"
#include "error.hpp"

Lexer::Lexer(int argc, char** argv)
{
    validate_argc_argv(argc, argv);
    m_source = source_to_string(argv[1]) + '\0';
}

std::vector<Token> Lexer::lex()
{
    while (peek() != '\0') {
        if (find_token_vt_map(std::string{peek()})) {
            eat();
        } else if (isalpha(peek())) {
            lex_ident_or_kw();
        } else if (isdigit(peek())) {
            lex_number();
        } else if (peek() == '"') {
            lex_string_lit();
        } else if (peek() == '#') {
            lex_comment();
        } else if (is_separator(peek())) {
            eat();
        } else {
            error("No matching token found for \"" << peek() << "\"");
        }
    }

    for (auto i : m_tokens)
        std::cout << i.m_value << "\n";

    m_tokens.push_back({Token_Type::END_OF_FILE, ""});
    m_buffer.clear();

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

void Lexer::validate_argc_argv(int argc, char** argv)
{
    assert(argc != 1);
    if (argv[1][strlen(argv[1]) - 6] != 'p' ||
        argv[1][strlen(argv[1]) - 5] != 's' ||
        argv[1][strlen(argv[1]) - 4] != 'e' ||
        argv[1][strlen(argv[1]) - 3] != 'u' ||
        argv[1][strlen(argv[1]) - 2] != 'd' ||
        argv[1][strlen(argv[1]) - 1] != 'o') {
        error("File lacks \".pseudo\" extension");
    }
}

std::string Lexer::source_to_string(char* file_name)
{
    std::ifstream file{};
    file.open(file_name);

    if (!file.is_open()) {
        error("File \"" << file_name << "\" could not be opened");
    }

    std::string source{std::istreambuf_iterator<char>(file),
                       std::istreambuf_iterator<char>()};

    file.close();

    return source;
}

bool Lexer::is_separator(char chr) const
{
    return chr == ' '  ||
           chr == '\n' ||
           chr == '\t' ||
           chr == '\0';
}

bool Lexer::find_token_vt_map(std::string value)
{
    auto got{value_token_map.find(value)};
    if (got != value_token_map.end()) {
        m_tokens.push_back({got->second, got->first});
        m_buffer.clear();
        return true;
    }

    return false;
}

void Lexer::lex_ident_or_kw()
{
    do {
        m_buffer += eat();
    } while (!is_separator(peek()) && isalnum(peek()));

    if (!find_token_vt_map(m_buffer)) {
        m_tokens.push_back({Token_Type::IDENTIFIER, m_buffer});
        m_buffer.clear();
    }
}

void Lexer::lex_number()
{
    Token_Type type{Token_Type::INT_LIT};

    do {
        m_buffer += eat();
        if (peek() == '.') {
            type = Token_Type::FLOAT;
        } else if (!isdigit(peek())) {
            break;
        }
    } while (!is_separator(peek()));

    m_tokens.push_back({type, m_buffer});
    m_buffer.clear();
}

void Lexer::lex_string_lit()
{
    do {
        m_buffer += eat();
    } while (peek() != '\0' && peek() != '"');

    m_tokens.push_back({Token_Type::STRING_LIT, m_buffer});
    m_buffer.clear();
}

void Lexer::lex_comment()
{
    do {
        eat();
    } while (peek() != '\0' && peek() != '\n');
}
