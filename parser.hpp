#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include "lexer.hpp"
#include "ast.hpp"

class Parser {
public:
	Parser(Lexer lexer);
	~Parser() = default;

	void parse();
	Stmt parse_stmt();
	VarStmt parse_vd_or_assignment();
	Expr parse_expr();
	IntExpr parse_int_expr();
	FloatExpr parse_float_expr();
	StrExpr parse_str_expr();
	AtomExpr parse_atom();
	BinOpExpr parse_bin_op_expr();
	bool is_bin_op(Token_Type type);
	Token peek(std::size_t dist = 0);
	Token eat(std::size_t dist = 1);
	Token try_eat(Token_Type type);
    Program& get_program();
    Error get_parse_error() const;
private:
    Error m_parse_error{};
    Program m_program{};
	std::vector<Token> m_tokens{};
	std::size_t m_index{};
};

#endif
