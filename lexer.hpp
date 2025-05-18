#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include <string>
#include <fstream>

enum class Token_Type {
    IDENTIFIER,
    INTEGER,
    FLOAT,
    SEMI,
    END_OF_FILE,
};

struct Token {
    Token_Type m_type{};
    std::string m_value{};
};

class Lexer {
public:
    Lexer(int argc, char** argv);
    ~Lexer() = default;
        
    std::vector<Token> lex();
    char peek(std::size_t dist = 0);
    char eat(std::size_t dist = 1);
    bool is_separator(char chr) const;
private:
    std::string m_source{};
    std::vector<Token> m_tokens{};
    std::size_t m_index{};
    std::string m_buffer{};
};

#endif
