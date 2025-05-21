#include <vector>
#include <cctype>
#include <fstream>
#include <cstring>
#include <iostream>
#include <cassert>
#include "lexer.hpp"

Lexer::Lexer(int argc, char** argv)
{
    m_file_name = argv[1];
    validate_argc_argv(argc, argv);
    m_source = source_to_string(argv[1]) + '\0';
}

std::vector<Token> Lexer::lex()
{
    while (peek() != '\0') {
        std::string peek_str{peek()};
        // TODO: change the name of the function
        if      (find_vt_map(peek_str)) eat();
        else if (isalpha(peek()))       lex_ident_or_kw();
        else if (isdigit(peek()))       lex_number();
        else if (peek() == '"')         lex_string_lit();
        else if (peek() == '#')         lex_comment();
        else if (is_separator(peek()))  eat();
        else lexer_error_lc("no matching token found for '" + std::string{peek()} + "'");
    }
    m_tokens.push_back({Token_Type::END_OF_FILE, ""});
    m_buffer.clear();
    return m_tokens;
}

char Lexer::peek(std::size_t dist)
{
    assert(m_index + dist <= m_source.size());
    return m_source.at(m_index + dist);
}

char Lexer::eat()
{
    assert(m_index + 1 <= m_source.size());
    // NOTE: counting current line and row for error reporting
    m_index++;
    if (peek() == '\n') {
        m_line++;
        m_col = 0;
    } else {
        m_col++;
    }
    return peek(-1);
}

void Lexer::validate_argc_argv(int argc, char** argv)
{
    assert(argc != 1);
    if (argv[1][strlen(argv[1]) - 6] != 'p' ||
        argv[1][strlen(argv[1]) - 5] != 's' ||
        argv[1][strlen(argv[1]) - 4] != 'e' ||
        argv[1][strlen(argv[1]) - 3] != 'u' ||
        argv[1][strlen(argv[1]) - 2] != 'd' ||
        argv[1][strlen(argv[1]) - 1] != 'o')
        lexer_error("file lacks '.pseudo' extension");
}

std::string Lexer::source_to_string(char* file_name)
{
    std::ifstream file{};
    file.open(file_name);
    if (!file.is_open()) lexer_error("file '" +
                                     std::string{file_name} +
                                     "' could not be opened");
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
    do    m_buffer += eat();
    while (!is_separator(peek()) && isalnum(peek()));
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
        if      (peek() == '.')    type = Token_Type::FLOAT;
        else if (!isdigit(peek())) break;
    } while (!is_separator(peek()));
    m_tokens.push_back({type, m_buffer});
    m_buffer.clear();
}

void Lexer::lex_string_lit()
{
    eat();
    do    m_buffer += eat();
    while (peek() != '\0' && peek() != '"');
    eat();
    m_tokens.push_back({Token_Type::STRING_LIT, m_buffer});
    m_buffer.clear();
}

void Lexer::lex_comment()
{
    do    eat();
    while (peek() != '\0' && peek() != '\n');
}

void Lexer::show_source_error()
{
    std::size_t line_char_count{std::to_string(m_line).size()};
    for (std::size_t i = line_char_count; i < 5; i++) std::cerr << " ";
    std::cerr << m_line << " | ";
    // NOTE: using start/end indexes to take a substring of the source
    std::size_t start{}, end{};
    for (std::size_t i = 0; i < m_line - 1; i++) {
        start = m_source.find("\n", start);
        start++;
    }
    end = m_source.find('\n', start);
    // NOTE: if there isn't a corresponding, \n, look for \0
    // HINT: might happen on the last line of the source
    if (end == std::string::npos) {
        end = m_source.find('\0', start);
    }
    std::cerr << m_source.substr(start, end - start) << "\n";
    // NOTE: alligns differently if line number is > 5 characters
    for (std::size_t i = 5; i < line_char_count; i++) std::cout << " ";
    std::cerr << "      | ";
    // NOTE: alligns the arrow so it points to the incorrect part of source
    for (std::size_t i = 0; i < m_col - 1; i++) {
        std::cerr << " ";
    }
    std::cerr << "^\n";
}

void Lexer::lexer_error(std::string message)
{
    std::cerr << m_file_name << ": error: " << message << "\n";
    exit(EXIT_FAILURE);
}

void Lexer::lexer_error_lc(std::string message)
{
    std::cerr << m_file_name << ":"         << m_line
                             << ":"         << m_col
                             << ": error: " << message << "\n";
    show_source_error();
    exit(EXIT_FAILURE);
}
