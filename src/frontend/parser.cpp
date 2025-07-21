#include <cassert>
#include <iostream>
#include <algorithm>
#include <variant>
#include <memory>

#include "parser.hpp"
#include "lexer.hpp"
#include "../utils/error.hpp"
#include "ast.hpp"

Parser::Parser(Lexer& lexer) : m_tokens(lexer.get_tokens()), m_source(lexer.get_source()) {}

void Parser::parse() { m_ast.m_body = parse_body(); }

[[nodiscard]] const AST& Parser::get_ast()                                  const { return m_ast; }
[[nodiscard]] const std::string& Parser::get_source()                       const { return m_source; }
[[nodiscard]] const std::vector<VarStmt>& Parser::get_existing_vars()       const { return m_existing_vars; }
[[nodiscard]] const std::vector<FuncDeclStmt>& Parser::get_existing_funcs() const { return m_existing_funcs; }

[[nodiscard]] Stmt Parser::parse_stmt()
{
	switch (peek().m_type) {
	case Token_Type::CONSTANT:
	case Token_Type::IDENTIFIER:  return Stmt{parse_var_stmt()};
	case Token_Type::OUTPUT:      return Stmt{parse_output_stmt()};
	case Token_Type::SUB_ROUTINE: return Stmt{parse_func_decl_stmt()};
	default:                      parse_error_l("error occurred while parsing statement");
	}
}

[[nodiscard]] VarStmt Parser::parse_var_stmt()
{
	VarStmt var_stmt{};

	if (peek().m_type == Token_Type::CONSTANT) {
		var_stmt.m_is_constant = true;
		eat();
	}

	var_stmt.m_name = try_eat(Token_Type::IDENTIFIER).m_value;

	if (!std::all_of(var_stmt.m_name.begin(), var_stmt.m_name.end(), [](char c) { return isupper(c); }) && var_stmt.m_is_constant) {
		parse_error_l("constants must have uppercase names");
	}

	try_eat(Token_Type::LESS_THAN);
	try_eat(Token_Type::MINUS);

	Token initial_expr_token{peek()};

	var_stmt.m_expr = std::make_shared<Expr>(parse_expr());

	if (is_var_defined(var_stmt)) {
		if (m_existing_vars.at(get_var_index(var_stmt)).m_is_constant) {
			parse_error_l("cannot reassign constants");
		}

		if (m_existing_vars.at(get_var_index(var_stmt)).m_expr->m_type != tt_to_dt(initial_expr_token)) {
			parse_error_l("can't reassign to a different type");
		}

		var_stmt.m_is_reassignment = true;
	} else {
		m_existing_vars.push_back(var_stmt);
	}

	return var_stmt;
}

[[nodiscard]] OutputStmt Parser::parse_output_stmt()
{
	OutputStmt output_stmt{};

	do {
		eat();
		output_stmt.m_args.m_exprs.push_back(parse_expr());
	} while (peek().m_type == Token_Type::COMMA);

	return output_stmt;
}

[[nodiscard]] FuncDeclStmt Parser::parse_func_decl_stmt()
{
	FuncDeclStmt func_decl_stmt{};

	eat(); // eat SUBROUTINE

	func_decl_stmt.m_name = try_eat(Token_Type::IDENTIFIER).m_value;

	m_var_scope_stack.push_back(m_existing_vars.size());

	try_eat(Token_Type::O_PAREN);
	func_decl_stmt.m_params = parse_func_decl_params();
	try_eat(Token_Type::C_PAREN);

	func_decl_stmt.m_body = std::make_unique<Body>(parse_body());

	if (peek().m_type == Token_Type::RETURN) {
		eat();
		func_decl_stmt.m_return.m_return_expr = std::make_shared<Expr>(parse_expr());
		func_decl_stmt.is_void = false;
	}

	try_eat(Token_Type::END_SUB_ROUTINE);

	m_existing_funcs.push_back(func_decl_stmt);

	m_existing_vars.resize(m_var_scope_stack.at(m_var_scope_stack.size() - 1));

	return func_decl_stmt;
}

[[nodiscard]] Params Parser::parse_func_decl_params()
{
	std::vector<Param> params{};

	while (peek().m_type != Token_Type::C_PAREN && peek().m_type != Token_Type::END_OF_FILE) {
		if (peek().m_type == Token_Type::COMMA) {
			eat();
		} else if (peek().m_type == Token_Type::IDENTIFIER) {
			params.push_back(Param{.m_name = peek().m_value});

			VarStmt var_stmt{};

			var_stmt.m_name = peek().m_value;
			var_stmt.m_expr = std::make_shared<Expr>(parse_atom());
			var_stmt.m_expr->m_type = Data_Type::NONE;

			m_existing_vars.push_back(var_stmt);

			eat();
		}
	}

	return Params{params};
}

[[nodiscard]] Args Parser::parse_func_call_args()
{
	std::vector<Expr> args{};

	while (peek().m_type != Token_Type::C_PAREN && peek().m_type != Token_Type::END_OF_FILE) {
		if (peek().m_type == Token_Type::COMMA) {
			eat();
		} else {
			args.push_back(Expr{parse_expr()});
		}
	}

	return Args{args};
}


[[nodiscard]] Body Parser::parse_body()
{
	std::vector<Stmt> stmts{};

	while (peek().m_type != Token_Type::END_SUB_ROUTINE && peek().m_type != Token_Type::END_OF_FILE &&
	       peek().m_type != Token_Type::RETURN) {
		stmts.push_back(parse_stmt());
	}

	return Body{stmts};
}

[[nodiscard]] Expr Parser::parse_expr()
{
	Expr expr{};

	// if type can be deduced, deduce it
	if (peek(1).m_type == Token_Type::O_PAREN) {
		expr.m_expr = parse_func_call_expr();
		return expr;
	} else {
		expr.m_type = deduce_expr_type(peek().m_type);
	}

	// in the case of a binary op...
	if (is_bin_op(peek(1).m_type)) {
		expr.m_expr = parse_bin_op_expr();
		return expr;
	}

	expr.m_expr = parse_atom();
	eat();

	return expr;
}

[[nodiscard]] Data_Type Parser::deduce_expr_type(Token_Type token_type)
{
	switch (token_type) {
	case (Token_Type::FLOAT):
	case (Token_Type::STRING_LIT): return tt_to_dt(peek().m_type);
	case (Token_Type::IDENTIFIER): return existing_vars_lookup(peek().m_value);
	// suppose we have a <- USERINPUT
	// in this case, we want `a` to be of type string by default
	case (Token_Type::USER_INPUT): return Data_Type::STRING;
	default:                       parse_error_l("couldn't match expression type");
	}
}

[[nodiscard]] FuncCallExpr Parser::parse_func_call_expr()
{
	FuncCallExpr func_call_expr{};

	func_call_expr.m_name = try_eat(Token_Type::IDENTIFIER).m_value;

	try_eat(Token_Type::O_PAREN);

	func_call_expr.m_args = parse_func_call_args();

	try_eat(Token_Type::C_PAREN);

	// might be useful one day
	if (is_func_defined(func_call_expr)) {
		if (is_args_length_matching(m_existing_funcs.at(get_func_index(func_call_expr)), func_call_expr)) {
			for (std::size_t i{0}; i < m_existing_funcs.at(get_func_index(func_call_expr)).m_params.m_params.size(); i++) {
				m_existing_funcs.at(get_func_index(func_call_expr)).m_params.m_params[i].m_type = func_call_expr.m_args.m_exprs[i].m_type;
			}
		} else {
			parse_error_l("argument number doesn't match w/ function definition");
		}
	} else {
		parse_error_l("function wasn't defined but used");
	}

	return func_call_expr;
}

[[nodiscard]] BinOpExpr Parser::parse_bin_op_expr()
{
	BinOpExpr bin_op_expr{};

	bin_op_expr.m_lhs = parse_lhs(peek());
	eat();

	bin_op_expr.m_op = tt_to_op(eat().m_type);

	if (is_bin_op(peek(1).m_type)) {
		bin_op_expr.m_rhs = parse_rhs(peek());
	} else {
		bin_op_expr.m_rhs = parse_lhs(peek());
		eat();
	}

	return bin_op_expr;
}

[[nodiscard]] AtomExpr Parser::parse_atom()
{
	switch (peek().m_type) {
	case Token_Type::FLOAT:      return AtomExpr{parse_float_expr()};
	case Token_Type::STRING_LIT: return AtomExpr{parse_str_expr()};
	case Token_Type::IDENTIFIER: return AtomExpr{parse_var_expr()};
	case Token_Type::USER_INPUT: return AtomExpr{parse_user_input_expr()};
	default:                     parse_error_l("atom couldn't be parsed");
	}
}

[[nodiscard]] IntExpr Parser::parse_int_expr()              { return IntExpr{std::stoi(peek().m_value)}; }
[[nodiscard]] FloatExpr Parser::parse_float_expr()          { return FloatExpr{std::stof(peek().m_value)}; }
[[nodiscard]] StrExpr Parser::parse_str_expr()              { return StrExpr{peek().m_value}; }
[[nodiscard]] VarExpr Parser::parse_var_expr()              { return VarExpr{peek().m_value}; }
[[nodiscard]] UserInputExpr Parser::parse_user_input_expr() { return UserInputExpr{}; }

[[nodiscard]] std::unique_ptr<Expr> Parser::parse_lhs(Token token)
{
	if (peek().m_type == Token_Type::IDENTIFIER) {
		return std::make_unique<Expr>(Expr{parse_atom(), existing_vars_lookup(token.m_value)});
	} else {
		return std::make_unique<Expr>(Expr{parse_atom(), tt_to_dt(token.m_type)});
	}
}

[[nodiscard]] std::unique_ptr<Expr> Parser::parse_rhs(Token token)
{
	if (peek().m_type == Token_Type::IDENTIFIER) {
		return std::make_unique<Expr>(Expr{parse_bin_op_expr(), existing_vars_lookup(token.m_value)});
	} else {
		return std::make_unique<Expr>(Expr{parse_bin_op_expr(), tt_to_dt(token.m_type)});
	}
}

[[nodiscard]] bool Parser::is_bin_op(Token_Type token_type)
{
	return token_type == Token_Type::PLUS     ||
	       token_type == Token_Type::MINUS    ||
	       token_type == Token_Type::MULTIPLY ||
	       token_type == Token_Type::DIVIDE   ||
	       token_type == Token_Type::DIV      ||
	       token_type == Token_Type::MOD;
}

[[nodiscard]] bool Parser::is_var_defined(const VarStmt& var_stmt)
{
	for (auto it{m_existing_vars.rbegin()}; it != m_existing_vars.rend(); it++) {
		if (it->m_name == var_stmt.m_name) {
			return true;
		}
	}

	return false;
}

[[nodiscard]] std::size_t Parser::get_var_index(const VarStmt& var_stmt)
{
	// make this cleaner
	std::size_t i{};

	for (auto it{m_existing_vars.rbegin()}; it != m_existing_vars.rend(); it++, i++) {
		if (it->m_name == var_stmt.m_name) {
			return m_existing_vars.size() - i - 1;
		}
	}

	parse_error_l("cannot get an index of a variable that doesn't exist");
}

[[nodiscard]] Data_Type Parser::existing_vars_lookup(std::string_view name)
{
	for (auto it{m_existing_vars.rbegin()}; it != m_existing_vars.rend(); it++) {
		if (it->m_name == name) {
			return it->m_expr->m_type;
		}
	}

	parse_error_l("couldn't match name with existing variables");
}

[[nodiscard]] bool Parser::is_func_defined(const FuncCallExpr& func_call_expr)
{
	for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++) {
		if (it->m_name == func_call_expr.m_name) {
			return true;
		}
	}

	return false;
}

[[nodiscard]] std::size_t Parser::get_func_index(const FuncCallExpr& func_call_expr)
{
	for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++) {
		if (it->m_name == func_call_expr.m_name) {
			return it - m_existing_funcs.begin();
		}
	}

	parse_error_l("cannot get an index of a function that doesn't exist");
}

[[nodiscard]] bool Parser::is_args_length_matching(const FuncDeclStmt& func_decl_stmt, const FuncCallExpr& func_call_expr)
{
	if (func_decl_stmt.m_params.m_params.size() == func_call_expr.m_args.m_exprs.size()){
		return true;
	}

	return false;
}


[[nodiscard]] Data_Type Parser::tt_to_dt(Token_Type token_type)
{
	switch (token_type) {
	case (Token_Type::FLOAT):      return Data_Type::DOUBLE;
	case (Token_Type::STRING_LIT): return Data_Type::STRING;
	default:                       parse_error_l("no match data type found");
	}
}

[[nodiscard]] Data_Type Parser::tt_to_dt(Token token)
{
	if (token.m_type == Token_Type::IDENTIFIER) {
		return existing_vars_lookup(token.m_value);
	} else {
		return tt_to_dt(token.m_type);
	}
}

[[nodiscard]] Operator Parser::tt_to_op(Token_Type token_type)
{
	switch (token_type) {
	case (Token_Type::PLUS):     return Operator::PLUS;
	case (Token_Type::MINUS):    return Operator::MINUS;
	case (Token_Type::MULTIPLY): return Operator::MULTIPLY;
	case (Token_Type::DIVIDE):   return Operator::DIVIDE;
	case (Token_Type::DIV):      return Operator::DIV;
	case (Token_Type::MOD):      return Operator::MOD;
	default:                     parse_error_l("no matching operator found");
	}
}

[[nodiscard]] Token Parser::peek(std::size_t distance)
{
	assert(m_token_index + distance <= m_tokens.size());
	return m_tokens.at(m_token_index + distance);
}

Token Parser::eat(std::size_t distance)
{
	assert(m_token_index + distance <= m_tokens.size());
	m_token_index += distance;
	return m_tokens.at(m_token_index - distance);
}

Token Parser::try_eat(Token_Type type)
{
	if (peek().m_type != type) {
		parse_error_s(
			"expected `" + to_string(type) + "` got `" + to_string(peek().m_type) + "`",
			peek().m_line,
			peek().m_col);
	} else {
		return eat();
	}
}
