#include <cassert>
#include "parser.hpp"
#include "lexer.hpp"
#include "error.hpp"
#include "ast.hpp"

Parser::Parser(Lexer lexer)
{
    m_tokens = lexer.get_tokens();
    m_file_name = lexer.get_file_name();
    m_source = lexer.get_source();
}

void Parser::parse()
{
    Program program{};
    while (peek().m_type != Token_Type::END_OF_FILE) {
        program.push_back(parse_stmt());
    }
}

Stmt Parser::parse_stmt()
{
    if (peek().m_type == Token_Type::IDENTIFIER) {
        return Stmt{parse_vd_or_assignment()};
    } else {
        eat();
    }
}

void Parser::parse_vd_or_assignment()
{
    // NOTE: eat identifier
    eat();
    // NOTE: <- symbol
    try_eat(Token_Type::LESS_THAN);
    try_eat(Token_Type::DASH);
}

Token Parser::peek(std::size_t dist)
{
    assert(m_index + dist <= m_tokens.size());
    return m_tokens.at(m_index + dist);
}

Token Parser::eat(std::size_t dist)
{
    assert(m_index + dist <= m_tokens.size());
    m_index += dist;
    return m_tokens.at(m_index - dist);
}

Token Parser::try_eat(Token_Type type)
{
    if (peek().m_type != type) {
        error_lc(m_source,
                 m_file_name,
                 "expected '" + to_string(type) + "' got '" + to_string(peek().m_type) + "'",
                 peek().m_line,
                 peek().m_col);
    } else {
        return eat();
    }
}
