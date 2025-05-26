#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include "lexer.hpp"

class Parser {
public:
    Parser(Lexer lexer);
    ~Parser() = default;

    void parse();
    void parse_stmt();
    void parse_vd_or_assignment();
    Token peek(std::size_t dist = 0);
    Token eat(std::size_t dist = 1);
    Token try_eat(Token_Type type);
private:
    std::vector<Token> m_tokens{};
    std::string m_file_name{};
    std::string m_source{};
    std::size_t m_index{};
};

#endif
