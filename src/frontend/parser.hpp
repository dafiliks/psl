#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <optional>
#include <utility>

#include "lexer.hpp"
#include "ast.hpp"

#define parse_error_s(a) \
	Error { a, peek().m_line, peek().m_col, m_source }
#define parse_error_l(a) \
	Error { a }

class Parser
{
public:
	Parser() = default;
	explicit Parser(Lexer &lexer);

	void parse();

	[[nodiscard]] const AST &get_ast() const;
	[[nodiscard]] const std::string &get_source() const;
	[[nodiscard]] const std::vector<VarStmt> &get_existing_vars() const;
	[[nodiscard]] const std::vector<FuncDeclStmt> &get_existing_funcs() const;

private:
	void populate_stdlib_funcs();

	[[nodiscard]] Stmt parse_stmt();
	[[nodiscard]] VarStmt parse_var_stmt();
	[[nodiscard]] OutputStmt parse_output_stmt();
	[[nodiscard]] FuncDeclStmt parse_func_decl_stmt();
	[[nodiscard]] FuncCallStmt parse_func_call_stmt();

	[[nodiscard]] RepeatUntilStmt parse_repeat_until_stmt();
	[[nodiscard]] WhileStmt parse_while_stmt();
	[[nodiscard]] IfStmt parse_if_stmt();
	[[nodiscard]] ElseIfStmt parse_else_if_stmt();
	[[nodiscard]] ElseStmt parse_else_stmt();
	[[nodiscard]] ForToStmt parse_for_to_stmt();

	void skip_over_function_body();

	[[nodiscard]] Params parse_func_decl_params();
	[[nodiscard]] Args parse_func_call_args();

	[[nodiscard]] Body parse_body_until(std::initializer_list<Token_Type> stop_tokens);

	void deduce_func_decl_param_types_from_expr(const FuncCallExpr &func_call_expr);
	void deduce_func_decl_param_types_from_stmt(const FuncCallStmt &func_call_stmt);

	[[nodiscard]] std::unique_ptr<Expr> parse_expr();
	[[nodiscard]] Data_Type deduce_expr_type();
	[[nodiscard]] FuncCallExpr parse_func_call_expr();
	[[nodiscard]] BinOpExpr parse_bin_op_expr();
	[[nodiscard]] AtomExpr parse_atom();
	[[nodiscard]] IntExpr parse_int_expr();
	[[nodiscard]] RealExpr parse_real_expr();
	[[nodiscard]] StrExpr parse_str_expr();

	[[nodiscard]] LenCallExpr parse_len_call_expr();
	[[nodiscard]] PositionCallExpr parse_position_call_expr();
	[[nodiscard]] SubStrCallExpr parse_sub_str_call_expr();
	[[nodiscard]] StrToIntCallExpr parse_str_to_int_call_expr();
	[[nodiscard]] StrToRealCallExpr parse_str_to_real_call_expr();
	[[nodiscard]] IntToStrCallExpr parse_int_to_str_call_expr();
	[[nodiscard]] RealToStrCallExpr parse_real_to_str_call_expr();
	[[nodiscard]] CharToCodeCallExpr parse_char_to_code_call_expr();
	[[nodiscard]] CodeToCharCallExpr parse_code_to_char_call_expr();
	[[nodiscard]] RandomIntCallExpr parse_random_int_call_expr();

	[[nodiscard]] VarExpr parse_var_expr();
	[[nodiscard]] UserInputExpr parse_user_input_expr();

	[[nodiscard]] std::unique_ptr<Expr> parse_lhs();
	[[nodiscard]] std::unique_ptr<Expr> parse_rhs();

	void parse_func_body_2nd_pass();
	void parse_unresolved_exprs_2nd_pass();

	[[nodiscard]] Operator determine_op();

	[[nodiscard]] bool is_bin_op(Token_Type token_type);
	[[nodiscard]] bool is_stmt(Token_Type token_type);
	[[nodiscard]] bool is_stdlib(const std::string_view name);
	[[nodiscard]] bool is_var_defined(const VarStmt &var_stmt);
	[[nodiscard]] Data_Type existing_vars_lookup(std::string_view name);
	[[nodiscard]] FuncDeclStmt &existing_func_lookup(std::string_view name);

	[[nodiscard]] Data_Type tt_to_dt(Token_Type token_type);
	[[nodiscard]] Data_Type tt_to_dt(Token token);
	[[nodiscard]] Operator tt_to_op(Token_Type token_type);

	[[nodiscard]] Token peek(std::size_t distance = 0);
	Token eat(int distance = 1);
	Token try_eat(Token_Type type);

	// private members
	AST m_ast{};
	std::vector<Token> m_tokens{};
	std::size_t m_token_index{};
	std::string m_source{};

	std::vector<VarStmt> m_existing_vars{};
	std::vector<FuncDeclStmt> m_existing_funcs{};

	std::vector<std::pair<Expr *, std::string>> m_unresolved_exprs{};
	std::string m_last_func_call_name{};

	std::vector<std::size_t> m_var_scope_stack{};
};

#endif
