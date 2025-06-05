#include <cassert>
#include <variant>
#include <memory>
#include "parser.hpp"
#include "lexer.hpp"
#include "error.hpp"
#include "ast.hpp"

Parser::Parser(Lexer lexer)
: m_tokens(lexer.get_tokens()),
  m_parse_error(lexer.get_lex_error()) {}

void Parser::parse()
{
	while (peek().m_type != Token_Type::END_OF_FILE) {
		m_program.m_body.push_back(parse_stmt());
	}
}

Stmt Parser::parse_stmt()
{
	if (peek().m_type == Token_Type::IDENTIFIER ||
	    peek().m_type == Token_Type::CONSTANT) {
		return Stmt{parse_vd_or_assignment()};
	} else {
		eat();
	}
}

VarStmt Parser::parse_vd_or_assignment()
{
	VarStmt var_stmt{};

	if (peek().m_type == Token_Type::CONSTANT) {
		var_stmt.m_is_constant = true;
		eat();
	}

	var_stmt.m_name = eat().m_value;
	try_eat(Token_Type::LESS_THAN);
	try_eat(Token_Type::DASH);
	var_stmt.m_expr = std::make_unique<Expr>(parse_expr());
	return var_stmt;
}

// TODO: convoluted function, refactor later
Expr Parser::parse_expr()
{
	Expr expr{};
	expr.m_type = Token_Type::STRING_LIT;
	if (peek().m_type == Token_Type::INT_LIT) {
		expr.m_type = Token_Type::INT_LIT;
	} else if (peek().m_type == Token_Type::FLOAT) {
		expr.m_type = Token_Type::FLOAT;
	}
	if (is_bin_op(peek(1).m_type)) {
		expr.m_expr = parse_bin_op_expr();
		return expr;
	}
	expr.m_expr = parse_atom();
	return expr;
}

IntExpr Parser::parse_int_expr()
{
	IntExpr int_expr{};
	int_expr.m_value = std::stoi(eat().m_value);
	return int_expr;
}

FloatExpr Parser::parse_float_expr()
{
	FloatExpr float_expr{};
	float_expr.m_value = std::stof(eat().m_value);
	return float_expr;
}

StrExpr Parser::parse_str_expr()
{
	StrExpr str_expr{};
	str_expr.m_value = eat().m_value;
	return str_expr;
}

AtomExpr Parser::parse_atom()
{
	switch (peek().m_type) {
	case Token_Type::INT_LIT:
		return AtomExpr{parse_int_expr()};
		break;
	case Token_Type::FLOAT:
		return AtomExpr{parse_float_expr()};
		break;
	case Token_Type::STRING_LIT:
		return AtomExpr{parse_str_expr()};
		break;
	}
}

// TODO: fix logic
BinOpExpr Parser::parse_bin_op_expr()
{
	BinOpExpr bin_op_expr{};
	bin_op_expr.m_lhs = std::make_unique<Expr>(Expr{parse_atom(), peek(-1).m_type});
	bin_op_expr.m_op = eat().m_type;
	if (is_bin_op(peek(1).m_type)) {
		Token_Type type = peek().m_type;
		bin_op_expr.m_rhs = std::make_unique<Expr>(Expr{parse_bin_op_expr(), type});
	} else {
		bin_op_expr.m_rhs = std::make_unique<Expr>(Expr{parse_atom(), peek(-1).m_type});
	}
	return bin_op_expr;
}

bool Parser::is_bin_op(Token_Type type)
{
	return type == Token_Type::PLUS     ||
	       type == Token_Type::MINUS    ||
	       type == Token_Type::MULTIPLY ||
	       type == Token_Type::DIVIDE;
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

// TODO: fix misalligned ^ symbol on errors. not important rn
Token Parser::try_eat(Token_Type type)
{
	if (peek().m_type != type) {
        m_parse_error.error_lc("expected '" + to_string(type) + "' got '" + to_string(peek().m_type) + "'",
                               peek().m_line,
                               peek().m_col);
	} else {
		return eat();
	}
}

Program& Parser::get_program()
{
    return m_program;
}

Error Parser::get_parse_error() const
{
    return m_parse_error;
}
