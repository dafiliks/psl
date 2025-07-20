#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include "lexer.hpp"
#include "ast.hpp"

#define parse_error_s(a, b, c) Error{a, b, c, m_source}
#define parse_error_l(a) Error{a}

class Parser {
public:
	Parser() = default;
	explicit Parser(Lexer& lexer);

	void parse();

	[[nodiscard]] const AST& get_ast() const;
	[[nodiscard]] const std::string& get_source() const;

private:
	[[nodiscard]] Stmt parse_stmt();
	[[nodiscard]] VarStmt parse_var_stmt();
	[[nodiscard]] OutputStmt parse_output_stmt();
	[[nodiscard]] FuncDeclStmt parse_func_decl_stmt();

	[[nodiscard]] Args parse_func_decl_args();
	[[nodiscard]] Body parse_body();

	[[nodiscard]] Expr parse_expr();
	[[nodiscard]] Data_Type deduce_expr_type(Token_Type token_type);
	[[nodiscard]] BinOpExpr parse_bin_op_expr();
	[[nodiscard]] AtomExpr parse_atom();
	[[nodiscard]] IntExpr parse_int_expr();
	[[nodiscard]] FloatExpr parse_float_expr();
	[[nodiscard]] StrExpr parse_str_expr();
	[[nodiscard]] VarExpr parse_var_expr();
	[[nodiscard]] UserInputExpr parse_user_input_expr();

	[[nodiscard]] std::unique_ptr<Expr> parse_lhs(Token token);
	[[nodiscard]] std::unique_ptr<Expr> parse_rhs(Token token);
	[[nodiscard]] bool is_bin_op(Token_Type token_type);

	[[nodiscard]] bool is_var_defined(VarStmt& var_stmt);
	[[nodiscard]] std::size_t get_var_index(VarStmt& var_stmt);
	[[nodiscard]] Data_Type existing_vars_lookup(std::string_view name);

	[[nodiscard]] Data_Type tt_to_dt(Token_Type token_type);
	[[nodiscard]] Data_Type tt_to_dt(Token token);
	[[nodiscard]] Operator tt_to_op(Token_Type token_type);

	[[nodiscard]] Token peek(std::size_t distance = 0);
	Token eat(std::size_t distance = 1);
	Token try_eat(Token_Type type);

	// private members
	AST m_ast{};
	std::vector<Token> m_tokens{};
	std::size_t m_token_index{};
	std::string m_source{};
	std::vector<VarStmt> m_existing_vars{};
};

#endif
