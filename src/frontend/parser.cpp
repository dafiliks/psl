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

void Parser::parse()
{
	m_ast.m_body = parse_body();
	parse_func_body_2nd_pass();
}

[[nodiscard]] const AST& Parser::get_ast()                                  const { return m_ast; }
[[nodiscard]] const std::string& Parser::get_source()                       const { return m_source; }
[[nodiscard]] const std::vector<VarStmt>& Parser::get_existing_vars()       const { return m_existing_vars; }
[[nodiscard]] const std::vector<FuncDeclStmt>& Parser::get_existing_funcs() const { return m_existing_funcs; }

[[nodiscard]] Stmt Parser::parse_stmt()
{
	if (peek().m_type == Token_Type::CONSTANT)
		return Stmt{parse_var_stmt()};

	if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type != Token_Type::O_PAREN)
		return Stmt{parse_var_stmt()};

	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type == Token_Type::O_PAREN)
		return Stmt{parse_func_call_stmt()};

	else if (peek().m_type == Token_Type::OUTPUT)
		return Stmt{parse_output_stmt()};

	else if (peek().m_type == Token_Type::SUB_ROUTINE)
		return Stmt{parse_func_decl_stmt()};

	else
		parse_error_l("error occurred while parsing statement");
}

[[nodiscard]] VarStmt Parser::parse_var_stmt()
{
	VarStmt var_stmt{};

	if (peek().m_type == Token_Type::CONSTANT) {
		var_stmt.m_is_constant = true;
		eat();
	}

	var_stmt.m_name = try_eat(Token_Type::IDENTIFIER).m_value;

	try_eat(Token_Type::LESS_THAN);
	try_eat(Token_Type::MINUS);

	var_stmt.m_expr = std::make_shared<Expr>(parse_expr());

	if (is_var_defined(var_stmt)) {
		// doesn't work yet
		var_stmt.m_previous_expr = var_stmt.m_expr;
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

void Parser::skip_over_function_body()
{
	while (peek().m_type != Token_Type::END_SUB_ROUTINE && peek().m_type != Token_Type::END_OF_FILE) {
		eat();
	}
}

[[nodiscard]] FuncDeclStmt Parser::parse_func_decl_stmt()
{
	FuncDeclStmt func_decl_stmt{};

	eat();

	func_decl_stmt.m_name = try_eat(Token_Type::IDENTIFIER).m_value;

	try_eat(Token_Type::O_PAREN);
	func_decl_stmt.m_params = parse_func_decl_params();
	try_eat(Token_Type::C_PAREN);

	func_decl_stmt.m_token_index_start = m_token_index;

	skip_over_function_body();

	try_eat(Token_Type::END_SUB_ROUTINE);

	m_existing_funcs.push_back(func_decl_stmt);

	return func_decl_stmt;
}

[[nodiscard]] FuncCallStmt Parser::parse_func_call_stmt()
{
	FuncCallStmt func_call_stmt{};

	func_call_stmt.m_name = eat().m_value;

	try_eat(Token_Type::O_PAREN);

	func_call_stmt.m_args = parse_func_call_args();

	deduce_func_decl_param_types_from_stmt(func_call_stmt);

	try_eat(Token_Type::C_PAREN);

	return func_call_stmt;
}

[[nodiscard]] Params Parser::parse_func_decl_params()
{
	std::vector<Param> params{};

	while (peek().m_type != Token_Type::C_PAREN &&
	       peek().m_type != Token_Type::END_OF_FILE) {
		switch (peek().m_type) {
		case (Token_Type::COMMA):
			eat();
			break;
		case (Token_Type::IDENTIFIER):
			params.push_back(Param{.m_name = peek().m_value});
			eat();
			break;
		default:
			parse_error_l("didn't recieve identifier as func decl parameter");
		}
	}

	return Params{params};
}

[[nodiscard]] Args Parser::parse_func_call_args()
{
	std::vector<Expr> args{};

	while (peek().m_type != Token_Type::C_PAREN &&
	       peek().m_type != Token_Type::END_OF_FILE) {
		switch (peek().m_type) {
		case (Token_Type::COMMA):
			eat();
			break;
		default:
			args.push_back(Expr{parse_expr()});
		}
	}

	return Args{args};
}


[[nodiscard]] Body Parser::parse_body()
{
	std::vector<Stmt> stmts{};

	while (peek().m_type != Token_Type::END_SUB_ROUTINE &&
	       peek().m_type != Token_Type::END_OF_FILE &&
	       peek().m_type != Token_Type::RETURN) {
		stmts.push_back(parse_stmt());
	}

	return Body{stmts};
}

[[nodiscard]] Expr Parser::parse_expr()
{
	Expr expr{};

	bool is_func_call{};

	std::size_t distance{};

	if (peek(1).m_type != Token_Type::O_PAREN) {
		expr.m_type = deduce_expr_type();
		is_func_call = false;
	} else {
		distance = existing_func_lookup(peek().m_value).m_params.m_params.size() * 2 + 1;
		eat(distance);
		is_func_call = true;
	}

	if (is_bin_op(peek(1).m_type)) {
		if (is_func_call) eat(-distance);
		expr.m_expr = parse_bin_op_expr();
		return expr;
	} else if (!is_bin_op(peek(1).m_type) && is_func_call) {
		eat(-distance);
	}

	expr.m_expr = parse_atom();
	eat();

	return expr;
}

[[nodiscard]] Data_Type Parser::deduce_expr_type()
{
	switch (peek().m_type) {
	case (Token_Type::FLOAT):
	case (Token_Type::STRING_LIT): return tt_to_dt(peek().m_type);
	case (Token_Type::IDENTIFIER): return existing_vars_lookup(peek().m_value);
	case (Token_Type::USER_INPUT): return Data_Type::STRING;
	default:                       parse_error_l("couldn't match expression type");
	}
}

void Parser::deduce_func_decl_param_types_from_expr(const FuncCallExpr& func_call_expr)
{
	FuncDeclStmt& func_decl_stmt{existing_func_lookup(func_call_expr.m_name)};

	if (!func_decl_stmt.m_is_called) {
		for (std::size_t i{0}; i < func_decl_stmt.m_params.m_params.size(); i++) {
			func_decl_stmt.m_params.m_params[i].m_type = func_call_expr.m_args.m_exprs[i].m_type;
		}
		func_decl_stmt.m_is_called = true;
	}
}

void Parser::deduce_func_decl_param_types_from_stmt(const FuncCallStmt& func_call_stmt)
{
	FuncDeclStmt& func_decl_stmt{existing_func_lookup(func_call_stmt.m_name)};

	if (!func_decl_stmt.m_is_called) {
		for (std::size_t i{0}; i < func_decl_stmt.m_params.m_params.size(); i++) {
			func_decl_stmt.m_params.m_params[i].m_type = func_call_stmt.m_args.m_exprs[i].m_type;
		}
		func_decl_stmt.m_is_called = true;
	}
}

[[nodiscard]] FuncCallExpr Parser::parse_func_call_expr()
{
	FuncCallExpr func_call_expr{};

	func_call_expr.m_name = try_eat(Token_Type::IDENTIFIER).m_value;

	try_eat(Token_Type::O_PAREN);

	func_call_expr.m_args = parse_func_call_args();

	deduce_func_decl_param_types_from_expr(func_call_expr);

	return func_call_expr;
}

[[nodiscard]] BinOpExpr Parser::parse_bin_op_expr()
{
	BinOpExpr bin_op_expr{};

	bin_op_expr.m_lhs = parse_lhs(peek());
	eat();

	bin_op_expr.m_op = tt_to_op(eat().m_type);

	std::size_t distance{1};

	if (peek(1).m_type == Token_Type::O_PAREN) {
		distance = existing_func_lookup(peek().m_value).m_params.m_params.size() * 2 + 2;
	}

	if (is_bin_op(peek(distance).m_type)) {
		bin_op_expr.m_rhs = parse_rhs(peek());
	} else {
		bin_op_expr.m_rhs = parse_lhs(peek());
		eat();
	}

	return bin_op_expr;
}

[[nodiscard]] AtomExpr Parser::parse_atom()
{
	if (peek().m_type == Token_Type::FLOAT)
		return AtomExpr{parse_float_expr()};

	else if (peek().m_type == Token_Type::STRING_LIT)
		return AtomExpr{parse_str_expr()};

	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type == Token_Type::O_PAREN)
		return AtomExpr{parse_func_call_expr()};

	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type != Token_Type::O_PAREN)
		return AtomExpr{parse_var_expr()};

	else if (peek().m_type == Token_Type::USER_INPUT)
		return AtomExpr{parse_user_input_expr()};

	else
		parse_error_l("atom couldn't be parsed");
}

[[nodiscard]] IntExpr Parser::parse_int_expr()              { return IntExpr{std::stoi(peek().m_value)}; }
[[nodiscard]] FloatExpr Parser::parse_float_expr()          { return FloatExpr{std::stof(peek().m_value)}; }
[[nodiscard]] StrExpr Parser::parse_str_expr()              { return StrExpr{peek().m_value}; }
[[nodiscard]] VarExpr Parser::parse_var_expr()              { return VarExpr{peek().m_value}; }
[[nodiscard]] UserInputExpr Parser::parse_user_input_expr() { return UserInputExpr{}; }

[[nodiscard]] std::unique_ptr<Expr> Parser::parse_lhs(Token token)
{
	if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type != Token_Type::O_PAREN)
		return std::make_unique<Expr>(Expr{parse_atom(), existing_vars_lookup(token.m_value)});

	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type == Token_Type::O_PAREN)
		return std::make_unique<Expr>(Expr{parse_atom()}); // don't assign type, deduce it on 2nd pass

	else
		return std::make_unique<Expr>(Expr{parse_atom(), tt_to_dt(token.m_type)});
}

[[nodiscard]] std::unique_ptr<Expr> Parser::parse_rhs(Token token)
{
	if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type != Token_Type::O_PAREN)
		return std::make_unique<Expr>(Expr{parse_bin_op_expr(), existing_vars_lookup(token.m_value)});

	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type == Token_Type::O_PAREN)
		return std::make_unique<Expr>(Expr{parse_bin_op_expr()}); // don't assign type, deduce it on 2nd pass

	else
		return std::make_unique<Expr>(Expr{parse_bin_op_expr(), tt_to_dt(token.m_type)});
}

void Parser::parse_func_body_2nd_pass()
{
	for (auto& i : m_existing_funcs) {
		m_token_index = i.m_token_index_start;

		// handle function scope
		m_var_scope_stack.push_back(m_existing_vars.size());

		for (const auto& j : i.m_params.m_params) {
			VarStmt var_stmt{};

			var_stmt.m_expr = std::make_shared<Expr>();
			var_stmt.m_name = j.m_name;
			var_stmt.m_expr->m_type = j.m_type;

			m_existing_vars.push_back(var_stmt);
		}

		i.m_body = std::make_shared<Body>(parse_body());

		if (peek().m_type == Token_Type::RETURN) {
			eat();
			i.m_return.m_return_expr = std::make_shared<Expr>(parse_expr());
			i.m_is_void = false;
			try_eat(Token_Type::END_SUB_ROUTINE);
		} else if (peek().m_type == Token_Type::END_SUB_ROUTINE) {
			eat();
		} else {
			parse_error_l("didn't recieve closing statement");
		}

		// handle function scope
		m_existing_vars.resize(m_var_scope_stack.back());

		m_var_scope_stack.pop_back();
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
	if (!m_var_scope_stack.empty()) {
		std::size_t stop_index = m_var_scope_stack.back();
		for (std::size_t i = m_existing_vars.size(); i-- > stop_index;) {
			if (m_existing_vars[i].m_name == var_stmt.m_name) {
			    return true;
			}
		}
	}
	return false;
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

[[nodiscard]] FuncDeclStmt& Parser::existing_func_lookup(std::string_view name)
{
	for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++) {
		if (it->m_name == name) {
			return *it;
		}
	}
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

Token Parser::eat(int distance)
{
	assert(m_token_index + distance <= m_tokens.size());
	m_token_index += distance;
	return m_tokens.at(m_token_index - distance);
}

Token Parser::try_eat(Token_Type type)
{
	if (peek().m_type != type) {
		parse_error_s("expected `" + to_string(type) + "` got `" + to_string(peek().m_type) + "`");
	} else {
		return eat();
	}
}
