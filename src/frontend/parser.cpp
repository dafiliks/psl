#include <cassert>
#include <iostream>
#include <algorithm>
#include <optional>
#include <variant>
#include <memory>

#include "parser.hpp"
#include "lexer.hpp"
#include "../utils/error.hpp"
#include "ast.hpp"

Parser::Parser(Lexer &lexer) : m_tokens(lexer.get_tokens()), m_source(lexer.get_source()) {}

void Parser::parse()
{
	populate_stdlib_funcs();

	m_ast.m_body = parse_body_until({Token_Type::END_OF_FILE});

	parse_func_body_2nd_pass();
	parse_unresolved_exprs_2nd_pass();
}

[[nodiscard]] const AST &Parser::get_ast() const { return m_ast; }
[[nodiscard]] const std::string &Parser::get_source() const { return m_source; }
[[nodiscard]] const std::vector<VarStmt> &Parser::get_existing_vars() const { return m_existing_vars; }
[[nodiscard]] const std::vector<FuncDeclStmt> &Parser::get_existing_funcs() const { return m_existing_funcs; }

void Parser::populate_stdlib_funcs()
{
	auto create_stdlib_func = [&](const std::string_view name, std::size_t param_size, Data_Type return_type)
	{
		FuncDeclStmt func{};
		func.m_name = name;
		for (std::size_t i{0}; i < param_size; i++)
		{
			func.m_params.m_params.emplace_back("");
		}
		func.m_return.m_return_expr = std::make_unique<Expr>();
		func.m_return.m_return_expr->m_type = return_type;
		m_existing_funcs.emplace_back(func);
	};

	create_stdlib_func("LEN", 1, Data_Type::INT);
	create_stdlib_func("POSITION", 2, Data_Type::INT);
	create_stdlib_func("SUBSTRING", 3, Data_Type::STRING);
	create_stdlib_func("STRING_TO_INT", 1, Data_Type::INT);
	create_stdlib_func("STRING_TO_REAL", 1, Data_Type::REAL);
	create_stdlib_func("INT_TO_STRING", 1, Data_Type::STRING);
	create_stdlib_func("REAL_TO_STRING", 1, Data_Type::STRING);
	create_stdlib_func("CHAR_TO_CODE", 1, Data_Type::INT);
	create_stdlib_func("CODE_TO_CHAR", 1, Data_Type::STRING);
	create_stdlib_func("RANDOM_INT", 2, Data_Type::INT);
}

[[nodiscard]] Stmt Parser::parse_stmt()
{
	if (peek().m_type == Token_Type::CONSTANT)
	{
		return Stmt{parse_var_stmt()};
	}
	else if (peek().m_type == Token_Type::REPEAT)
	{
		return Stmt{parse_repeat_until_stmt()};
	}
	else if (peek().m_type == Token_Type::WHILE)
	{
		return Stmt{parse_while_stmt()};
	}
	else if (peek().m_type == Token_Type::IF)
	{
		return Stmt{parse_if_stmt()};
	}
	else if (peek().m_type == Token_Type::ELSE && peek(1).m_type != Token_Type::IF)
	{
		return Stmt{parse_else_stmt()};
	}
	else if (peek().m_type == Token_Type::ELSE && peek(1).m_type == Token_Type::IF)
	{
		return Stmt{parse_else_if_stmt()};
	}
	else if (peek().m_type == Token_Type::FOR && peek(2).m_type == Token_Type::IN)
	{
		return Stmt{parse_for_in_stmt()};
	}
	else if (peek().m_type == Token_Type::FOR && peek(2).m_type != Token_Type::IN)
	{
		return Stmt{parse_for_to_stmt()};
	}
	else if (peek().m_type == Token_Type::RECORD)
	{
		return Stmt{parse_record_stmt()};
	}
	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type == Token_Type::COLON)
	{
		return Stmt{parse_field_stmt()};
	}
	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type == Token_Type::SQ_O_BRACKET)
	{
		return Stmt{parse_list_access_stmt()};
	}
	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type != Token_Type::O_PAREN)
	{
		return Stmt{parse_var_stmt()};
	}
	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type == Token_Type::O_PAREN)
	{
		return Stmt{parse_func_call_stmt()};
	}
	else if (peek().m_type == Token_Type::OUTPUT)
	{
		return Stmt{parse_output_stmt()};
	}
	else if (peek().m_type == Token_Type::SUB_ROUTINE)
	{
		return Stmt{parse_func_decl_stmt()};
	}
	else
	{
		parse_error_l("error occurred while parsing statement");
	}
}

[[nodiscard]] VarStmt Parser::parse_var_stmt()
{
	VarStmt var_stmt{};

	if (peek().m_type == Token_Type::CONSTANT)
	{
		var_stmt.m_is_constant = true;
		eat();
	}

	var_stmt.m_name = try_eat(Token_Type::IDENTIFIER).m_value;

	try_eat(Token_Type::LESS_THAN);
	try_eat(Token_Type::MINUS);

	if (peek().m_type == Token_Type::SQ_O_BRACKET)
	{
		var_stmt.m_is_1d_list = true;
		if (peek(1).m_type == Token_Type::SQ_O_BRACKET)
		{
			var_stmt.m_is_2d_list = true;
		}

		if (peek(2).m_type == Token_Type::SQ_O_BRACKET)
		{
			parse_error_l("cannot have higher dimension arrays than 2d");
		}
	}

	var_stmt.m_expr = parse_expr();

	if (is_var_defined(var_stmt))
	{
		// doesn't work yet
		var_stmt.m_previous_expr = var_stmt.m_expr;
		var_stmt.m_is_reassignment = true;
	}
	else
	{
		m_existing_vars.push_back(var_stmt);
	}

	return var_stmt;
}

[[nodiscard]] OutputStmt Parser::parse_output_stmt()
{
	OutputStmt output_stmt{};

	do
	{
		eat();
		output_stmt.m_args.m_exprs.push_back(*parse_expr());
	} while (peek().m_type == Token_Type::COMMA);

	return output_stmt;
}

void Parser::skip_over_function_body()
{
	while (peek().m_type != Token_Type::END_SUB_ROUTINE && peek().m_type != Token_Type::END_OF_FILE)
	{
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

[[nodiscard]] RepeatUntilStmt Parser::parse_repeat_until_stmt()
{
	RepeatUntilStmt repeat_until_stmt{};

	try_eat(Token_Type::REPEAT);

	repeat_until_stmt.m_body = std::make_shared<Body>(parse_body_until({Token_Type::UNTIL}));

	try_eat(Token_Type::UNTIL);

	repeat_until_stmt.m_condition_expr = parse_expr();

	return repeat_until_stmt;
}

[[nodiscard]] WhileStmt Parser::parse_while_stmt()
{
	WhileStmt while_stmt{};

	try_eat(Token_Type::WHILE);

	while_stmt.m_condition_expr = parse_expr();

	while_stmt.m_body = std::make_shared<Body>(parse_body_until({Token_Type::END_WHILE}));

	try_eat(Token_Type::END_WHILE);

	return while_stmt;
}

[[nodiscard]] IfStmt Parser::parse_if_stmt()
{
	IfStmt if_stmt{};

	try_eat(Token_Type::IF);

	if_stmt.m_condition_expr = parse_expr();

	try_eat(Token_Type::THEN);

	if_stmt.m_body = std::make_shared<Body>(parse_body_until({Token_Type::ELSE, Token_Type::END_IF}));

	if (peek().m_type == Token_Type::END_IF)
	{
		eat();
	}

	return if_stmt;
}

[[nodiscard]] ElseIfStmt Parser::parse_else_if_stmt()
{
	ElseIfStmt else_if_stmt{};

	try_eat(Token_Type::ELSE);
	try_eat(Token_Type::IF);

	else_if_stmt.m_condition_expr = parse_expr();

	try_eat(Token_Type::THEN);

	else_if_stmt.m_body = std::make_shared<Body>(parse_body_until({Token_Type::ELSE, Token_Type::END_IF}));

	return else_if_stmt;
}

[[nodiscard]] ElseStmt Parser::parse_else_stmt()
{
	ElseStmt else_stmt{};

	try_eat(Token_Type::ELSE);

	else_stmt.m_body = std::make_shared<Body>(parse_body_until({Token_Type::END_IF}));

	try_eat(Token_Type::END_IF);

	return else_stmt;
}

[[nodiscard]] FieldStmt Parser::parse_field_stmt()
{
	FieldStmt field_stmt{};

	field_stmt.m_name = try_eat(Token_Type::IDENTIFIER).m_value;

	try_eat(Token_Type::COLON);

	field_stmt.m_type = tt_to_dt(eat().m_type);

	return field_stmt;
}

[[nodiscard]] RecordStmt Parser::parse_record_stmt()
{
	RecordStmt record_stmt{};

	try_eat(Token_Type::RECORD);

	record_stmt.m_name = try_eat(Token_Type::IDENTIFIER).m_value;

	record_stmt.m_fields = parse_fields_until({Token_Type::END_RECORD});

	try_eat(Token_Type::END_RECORD);

	return record_stmt;
}

[[nodiscard]] ForToStmt Parser::parse_for_to_stmt()
{
	ForToStmt for_to_stmt{};

	try_eat(Token_Type::FOR);

	for_to_stmt.m_var_stmt = parse_var_stmt();

	try_eat(Token_Type::TO);

	for_to_stmt.m_boundary = parse_expr();

	if (peek().m_type == Token_Type::STEP)
	{
		eat();
		for_to_stmt.m_step = parse_expr();
	}

	for_to_stmt.m_body = std::make_shared<Body>(parse_body_until({Token_Type::END_FOR}));

	try_eat(Token_Type::END_FOR);

	return for_to_stmt;
}

// try eat
[[nodiscard]] ForInStmt Parser::parse_for_in_stmt()
{
	ForInStmt for_in_stmt{};

	eat();

	for_in_stmt.m_declaration = eat().m_value;

	try_eat(Token_Type::IN);

	for_in_stmt.m_range = std::make_shared<Expr>(*parse_expr());

	VarStmt var_stmt{};
	var_stmt.m_name = for_in_stmt.m_declaration;
	var_stmt.m_expr = std::make_shared<Expr>();
	var_stmt.m_expr->m_type = for_in_stmt.m_range->m_type;
	m_existing_vars.push_back(var_stmt);

	for_in_stmt.m_body = std::make_shared<Body>(parse_body_until({Token_Type::END_FOR}));

	try_eat(Token_Type::END_FOR);

	return for_in_stmt;
}

[[nodiscard]] ListAccessStmt Parser::parse_list_access_stmt()
{
	ListAccessStmt list_access_stmt{};

	list_access_stmt.m_name = eat().m_value;

	eat();

	list_access_stmt.m_row = parse_expr();

	try_eat(Token_Type::SQ_C_BRACKET);

	if (existing_vars_lookup(list_access_stmt.m_name).m_is_2d_list && peek().m_type == Token_Type::SQ_O_BRACKET)
	{
		try_eat(Token_Type::SQ_O_BRACKET);

		list_access_stmt.m_col = parse_expr();

		try_eat(Token_Type::SQ_C_BRACKET);
	}

	try_eat(Token_Type::LESS_THAN);
	try_eat(Token_Type::MINUS);

	list_access_stmt.m_expr = parse_expr();

	return list_access_stmt;
}

[[nodiscard]] Params Parser::parse_func_decl_params()
{
	std::vector<Param> params{};

	while (peek().m_type != Token_Type::C_PAREN &&
		   peek().m_type != Token_Type::END_OF_FILE)
	{
		switch (peek().m_type)
		{
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
		   peek().m_type != Token_Type::END_OF_FILE)
	{
		switch (peek().m_type)
		{
		case (Token_Type::COMMA):
			eat();
			break;
		default:
			args.push_back(*parse_expr());
		}
	}

	return Args{args};
}

[[nodiscard]] Body Parser::parse_body_until(std::initializer_list<Token_Type> stop_tokens)
{
	std::vector<Stmt> stmts{};

	while (std::find(stop_tokens.begin(), stop_tokens.end(), peek().m_type) == stop_tokens.end() &&
		   peek().m_type != Token_Type::END_OF_FILE)
	{
		stmts.push_back(parse_stmt());
	}

	return Body{stmts};
}

[[nodiscard]] std::vector<FieldStmt> Parser::parse_fields_until(std::initializer_list<Token_Type> stop_tokens)
{
	std::vector<FieldStmt> fields{};

	while (std::find(stop_tokens.begin(), stop_tokens.end(), peek().m_type) == stop_tokens.end() &&
		   peek().m_type != Token_Type::END_OF_FILE)
	{
		fields.push_back(parse_field_stmt());
	}

	return fields;
}

[[nodiscard]] std::unique_ptr<Expr> Parser::parse_expr()
{
	std::unique_ptr<Expr> lhs{std::make_unique<Expr>()};

	if (peek().m_type == Token_Type::SQ_O_BRACKET)
	{
		ListExpr list_expr{};

		do {
			eat();
			list_expr.m_exprs.push_back(*parse_expr());
		} while (peek().m_type == Token_Type::COMMA);

		*lhs = Expr{list_expr, lhs->m_type};

		try_eat(Token_Type::SQ_C_BRACKET);
	}
	else if (peek().m_type == Token_Type::O_PAREN)
	{
		eat();

		ParenExpr paren_expr{};
		paren_expr.m_expr = std::make_shared<Expr>(*parse_expr());

		*lhs = Expr{paren_expr, lhs->m_type};

		try_eat(Token_Type::C_PAREN);
	}
	else if (is_unary(peek().m_type))
	{
		*lhs = Expr{parse_unary_op_expr(), lhs->m_type};
	}
	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type == Token_Type::O_PAREN)
	{
		m_unresolved_exprs.emplace_back(lhs.get(), peek().m_value);
		*lhs = Expr{AtomExpr{parse_func_call_expr()}, lhs->m_type};
	}
	else
	{
		lhs->m_type = deduce_expr_type(peek());
		*lhs = Expr{parse_atom(), lhs->m_type};
	}

	while (is_bin_op(peek().m_type))
	{
		BinOpExpr bin_op_expr{};
		bin_op_expr.m_lhs = std::make_shared<Expr>(*lhs);
		bin_op_expr.m_rhs = std::make_shared<Expr>();

		bin_op_expr.m_op = determine_op();

		if (is_unary(peek().m_type)) {
			bin_op_expr.m_rhs = std::make_shared<Expr>(parse_unary_op_expr());
		} else {
			bin_op_expr.m_rhs->m_type = deduce_expr_type(peek());
			bin_op_expr.m_rhs = std::make_shared<Expr>(parse_atom());
		}

		*lhs = Expr{bin_op_expr, lhs->m_type};
	}

	return lhs;
}

[[nodiscard]] Data_Type Parser::deduce_expr_type(Token token)
{
	if (token.m_type == Token_Type::REAL || token.m_type == Token_Type::INT ||
	    token.m_type == Token_Type::STRING || token.m_type == Token_Type::NOT)
	{
		return tt_to_dt(token.m_type);
	}
	else if (token.m_type == Token_Type::IDENTIFIER)
	{
		return existing_vars_lookup(token.m_value).m_expr->m_type;
	}
	else if (token.m_type == Token_Type::USER_INPUT)
	{
		return Data_Type::STRING;
	}
	else
	{
		parse_error_l("couldn't match expression type");
	}
}

void Parser::deduce_func_decl_param_types_from_expr(const FuncCallExpr &func_call_expr)
{
	FuncDeclStmt &func_decl_stmt{existing_func_lookup(func_call_expr.m_name)};

	if (!func_decl_stmt.m_is_called)
	{
		for (std::size_t i{0}; i < func_decl_stmt.m_params.m_params.size(); i++)
		{
			func_decl_stmt.m_params.m_params[i].m_type = func_call_expr.m_args.m_exprs[i].m_type;
		}
		func_decl_stmt.m_is_called = true;
	}
}

void Parser::deduce_func_decl_param_types_from_stmt(const FuncCallStmt &func_call_stmt)
{
	FuncDeclStmt &func_decl_stmt{existing_func_lookup(func_call_stmt.m_name)};

	if (!func_decl_stmt.m_is_called)
	{
		for (std::size_t i{0}; i < func_decl_stmt.m_params.m_params.size(); i++)
		{
			func_decl_stmt.m_params.m_params[i].m_type = func_call_stmt.m_args.m_exprs[i].m_type;
		}
		func_decl_stmt.m_is_called = true;
	}
}

[[nodiscard]] FieldAccessExpr Parser::parse_field_access_expr()
{
	FieldAccessExpr field_access_expr{};

	field_access_expr.m_record_name = try_eat(Token_Type::IDENTIFIER).m_value;

	try_eat(Token_Type::DOT);

	field_access_expr.m_field_name = try_eat(Token_Type::IDENTIFIER).m_value;

	return field_access_expr;
}

[[nodiscard]] ListAccessExpr Parser::parse_list_access_expr()
{
	ListAccessExpr list_access_expr{};

	list_access_expr.m_name = eat().m_value;

	eat();

	list_access_expr.m_row = parse_expr();

	try_eat(Token_Type::SQ_C_BRACKET);

	if (existing_vars_lookup(list_access_expr.m_name).m_is_2d_list && peek().m_type == Token_Type::SQ_O_BRACKET)
	{
		try_eat(Token_Type::SQ_O_BRACKET);

		list_access_expr.m_col = parse_expr();

		try_eat(Token_Type::SQ_C_BRACKET);
	}

	return list_access_expr;
}

[[nodiscard]] FuncCallExpr Parser::parse_func_call_expr()
{
	FuncCallExpr func_call_expr{};

	func_call_expr.m_name = try_eat(Token_Type::IDENTIFIER).m_value;

	try_eat(Token_Type::O_PAREN);

	func_call_expr.m_args = parse_func_call_args();

	deduce_func_decl_param_types_from_expr(func_call_expr);

	try_eat(Token_Type::C_PAREN);

	return func_call_expr;
}

[[nodiscard]] UnaryOpExpr Parser::parse_unary_op_expr()
{
	UnaryOpExpr unary_op_expr{};

	unary_op_expr.m_op = determine_op();

	unary_op_expr.m_unary_expr = std::make_shared<Expr>(*parse_expr());

	return unary_op_expr;
}

[[nodiscard]] AtomExpr Parser::parse_atom()
{
	if (peek().m_type == Token_Type::REAL)
	{
		return AtomExpr{parse_real_expr()};
	}
	else if (peek().m_type == Token_Type::INT)
	{
		return AtomExpr{parse_int_expr()};
	}
	else if (peek().m_type == Token_Type::STRING)
	{
		return AtomExpr{parse_str_expr()};
	}
	else if (peek().m_type == Token_Type::LEN)
	{
		return AtomExpr{parse_len_call_expr()};
	}
	else if (peek().m_type == Token_Type::POSITION)
	{
		return AtomExpr{parse_position_call_expr()};
	}
	else if (peek().m_type == Token_Type::SUBSTRING)
	{
		return AtomExpr{parse_sub_str_call_expr()};
	}
	else if (peek().m_type == Token_Type::STRING_TO_INT)
	{
		return AtomExpr{parse_str_to_int_call_expr()};
	}
	else if (peek().m_type == Token_Type::STRING_TO_REAL)
	{
		return AtomExpr{parse_str_to_real_call_expr()};
	}
	else if (peek().m_type == Token_Type::INT_TO_STRING)
	{
		return AtomExpr{parse_int_to_str_call_expr()};
	}
	else if (peek().m_type == Token_Type::REAL_TO_STRING)
	{
		return AtomExpr{parse_real_to_str_call_expr()};
	}
	else if (peek().m_type == Token_Type::CHAR_TO_CODE)
	{
		return AtomExpr{parse_char_to_code_call_expr()};
	}
	else if (peek().m_type == Token_Type::CODE_TO_CHAR)
	{
		return AtomExpr{parse_code_to_char_call_expr()};
	}
	else if (peek().m_type == Token_Type::RANDOM_INT)
	{
		return AtomExpr{parse_random_int_call_expr()};
	}
	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type == Token_Type::DOT)
	{
		return AtomExpr{parse_field_access_expr()};
	}
	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type == Token_Type::SQ_O_BRACKET)
	{
		return AtomExpr{parse_list_access_expr()};
	}
	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type == Token_Type::O_PAREN)
	{
		return AtomExpr{parse_func_call_expr()};
	}
	else if (peek().m_type == Token_Type::IDENTIFIER && peek(1).m_type != Token_Type::O_PAREN)
	{
		return AtomExpr{parse_var_expr()};
	}
	else if (peek().m_type == Token_Type::USER_INPUT)
	{
		return AtomExpr{parse_user_input_expr()};
	}
	else
	{
		parse_error_l("atom couldn't be parsed");
	}
}

[[nodiscard]] IntExpr Parser::parse_int_expr()
{
	return IntExpr{std::stoi(try_eat(Token_Type::INT).m_value)};
}

[[nodiscard]] RealExpr Parser::parse_real_expr()
{
	return RealExpr{std::stof(try_eat(Token_Type::REAL).m_value)};
}

[[nodiscard]] StrExpr Parser::parse_str_expr()
{
	return StrExpr{try_eat(Token_Type::STRING).m_value};
}

[[nodiscard]] LenCallExpr Parser::parse_len_call_expr()
{
	LenCallExpr len_call_expr{};

	try_eat(Token_Type::LEN);
	try_eat(Token_Type::O_PAREN);

	len_call_expr.m_expr = parse_expr();

	try_eat(Token_Type::C_PAREN);

	return len_call_expr;
}

[[nodiscard]] PositionCallExpr Parser::parse_position_call_expr()
{
	PositionCallExpr position_call_expr{};

	try_eat(Token_Type::POSITION);
	try_eat(Token_Type::O_PAREN);

	position_call_expr.m_str_expr = parse_expr();

	try_eat(Token_Type::COMMA);

	position_call_expr.m_char_expr = parse_expr();

	try_eat(Token_Type::C_PAREN);

	return position_call_expr;
}

[[nodiscard]] SubStrCallExpr Parser::parse_sub_str_call_expr()
{
	SubStrCallExpr sub_str_call_expr{};

	try_eat(Token_Type::SUBSTRING);
	try_eat(Token_Type::O_PAREN);

	sub_str_call_expr.m_num1_expr = parse_expr();

	try_eat(Token_Type::COMMA);

	sub_str_call_expr.m_num2_expr = parse_expr();

	try_eat(Token_Type::COMMA);

	sub_str_call_expr.m_str_expr = parse_expr();

	try_eat(Token_Type::C_PAREN);

	return sub_str_call_expr;
}

[[nodiscard]] StrToIntCallExpr Parser::parse_str_to_int_call_expr()
{
	StrToIntCallExpr str_to_int_call_expr{};

	try_eat(Token_Type::STRING_TO_INT);
	try_eat(Token_Type::O_PAREN);

	str_to_int_call_expr.m_str_expr = parse_expr();

	try_eat(Token_Type::C_PAREN);

	return str_to_int_call_expr;
}

[[nodiscard]] StrToRealCallExpr Parser::parse_str_to_real_call_expr()
{
	StrToRealCallExpr str_to_real_call_expr{};

	try_eat(Token_Type::STRING_TO_REAL);
	try_eat(Token_Type::O_PAREN);

	str_to_real_call_expr.m_str_expr = parse_expr();

	try_eat(Token_Type::C_PAREN);

	return str_to_real_call_expr;
}

[[nodiscard]] IntToStrCallExpr Parser::parse_int_to_str_call_expr()
{
	IntToStrCallExpr int_to_str_call_expr{};

	try_eat(Token_Type::INT_TO_STRING);
	try_eat(Token_Type::O_PAREN);

	int_to_str_call_expr.m_int_expr = parse_expr();

	try_eat(Token_Type::C_PAREN);

	return int_to_str_call_expr;
}

[[nodiscard]] RealToStrCallExpr Parser::parse_real_to_str_call_expr()
{
	RealToStrCallExpr real_to_str_call_expr{};

	try_eat(Token_Type::REAL_TO_STRING);
	try_eat(Token_Type::O_PAREN);

	real_to_str_call_expr.m_real_expr = parse_expr();

	try_eat(Token_Type::C_PAREN);

	return real_to_str_call_expr;
}

[[nodiscard]] CharToCodeCallExpr Parser::parse_char_to_code_call_expr()
{
	CharToCodeCallExpr char_to_code_call_expr{};

	try_eat(Token_Type::CHAR_TO_CODE);
	try_eat(Token_Type::O_PAREN);

	char_to_code_call_expr.m_char_expr = parse_expr();

	try_eat(Token_Type::C_PAREN);

	return char_to_code_call_expr;
}

[[nodiscard]] CodeToCharCallExpr Parser::parse_code_to_char_call_expr()
{
	CodeToCharCallExpr code_to_char_call_expr{};

	try_eat(Token_Type::CODE_TO_CHAR);
	try_eat(Token_Type::O_PAREN);

	code_to_char_call_expr.m_int_expr = parse_expr();

	try_eat(Token_Type::C_PAREN);

	return code_to_char_call_expr;
}

[[nodiscard]] RandomIntCallExpr Parser::parse_random_int_call_expr()
{
	RandomIntCallExpr random_int_call_expr{};

	try_eat(Token_Type::RANDOM_INT);
	try_eat(Token_Type::O_PAREN);

	random_int_call_expr.m_int1_expr = parse_expr();

	try_eat(Token_Type::COMMA);

	random_int_call_expr.m_int2_expr = parse_expr();

	try_eat(Token_Type::C_PAREN);

	return random_int_call_expr;
}

[[nodiscard]] VarExpr Parser::parse_var_expr()
{
	return VarExpr{try_eat(Token_Type::IDENTIFIER).m_value};
}
[[nodiscard]] UserInputExpr Parser::parse_user_input_expr()
{
	try_eat(Token_Type::USER_INPUT);
	return UserInputExpr{};
}

void Parser::parse_func_body_2nd_pass()
{
	for (auto &i : m_existing_funcs)
	{
		if (!is_stdlib(i.m_name))
		{
			m_token_index = i.m_token_index_start;

			m_var_scope_stack.push_back(m_existing_vars.size());

			for (const auto &j : i.m_params.m_params)
			{
				VarStmt var_stmt{};

				var_stmt.m_expr = std::make_shared<Expr>();
				var_stmt.m_name = j.m_name;
				var_stmt.m_expr->m_type = j.m_type;

				m_existing_vars.push_back(var_stmt);
			}

			i.m_body = std::make_shared<Body>(parse_body_until({Token_Type::RETURN, Token_Type::END_SUB_ROUTINE}));

			if (peek().m_type == Token_Type::RETURN)
			{
				eat();
				i.m_return.m_return_expr = std::make_shared<Expr>(*parse_expr());
				i.m_is_void = false;
				try_eat(Token_Type::END_SUB_ROUTINE);
			}
			else if (peek().m_type == Token_Type::END_SUB_ROUTINE)
			{
				eat();
			}
			else
			{
				parse_error_l("didn't recieve closing statement");
			}

			m_existing_vars.resize(m_var_scope_stack.back());
			m_var_scope_stack.pop_back();
		}
	}
}

void Parser::parse_unresolved_exprs_2nd_pass()
{
	for (auto &i : m_unresolved_exprs)
	{
		if (!existing_func_lookup(i.second).m_is_void)
		{
			i.first->m_type = existing_func_lookup(i.second).m_return.m_return_expr->m_type;
		}
		else
		{
			parse_error_l("cannot use void func in expr");
		}
	}
}

[[nodiscard]] Operator Parser::determine_op()
{
	if (peek(1).m_type == Token_Type::END_OF_FILE)
	{
		parse_error_l("eof reached when looking for relational op");
	}

	if (peek().m_type == Token_Type::PLUS)
	{
		eat();
		return Operator::PLUS;
	}
	else if (peek().m_type == Token_Type::MINUS)
	{
		eat();
		return Operator::MINUS;
	}
	else if (peek().m_type == Token_Type::MULTIPLY)
	{
		eat();
		return Operator::MULTIPLY;
	}
	else if (peek().m_type == Token_Type::DIVIDE)
	{
		eat();
		return Operator::DIVIDE;
	}
	else if (peek().m_type == Token_Type::DIV)
	{
		eat();
		return Operator::DIV;
	}
	else if (peek().m_type == Token_Type::MOD)
	{
		eat();
		return Operator::MOD;
	}
	else if (peek().m_type == Token_Type::LESS_THAN && peek(1).m_type != Token_Type::EQUALS)
	{
		eat();
		return Operator::LESS_THAN;
	}
	else if (peek().m_type == Token_Type::LESS_THAN && peek(1).m_type == Token_Type::EQUALS)
	{
		eat(2);
		return Operator::LESS_THAN_OET;
	}
	else if (peek().m_type == Token_Type::GREATER_THAN && peek(1).m_type != Token_Type::EQUALS)
	{
		eat();
		return Operator::GREATER_THAN;
	}
	else if (peek().m_type == Token_Type::GREATER_THAN && peek(1).m_type == Token_Type::EQUALS)
	{
		eat(2);
		return Operator::GREATER_THAN_OET;
	}
	else if (peek().m_type == Token_Type::EXCLAIMATION && peek(1).m_type == Token_Type::EQUALS)
	{
		eat(2);
		return Operator::NOT_EQUALS;
	}
	else if (peek().m_type == Token_Type::EQUALS)
	{
		eat();
		return Operator::EQUALS;
	}
	else if (peek().m_type == Token_Type::AND)
	{
		eat();
		return Operator::AND;
	}
	else if (peek().m_type == Token_Type::OR)
	{
		eat();
		return Operator::OR;
	}
	else if (peek().m_type == Token_Type::NOT)
	{
		eat();
		return Operator::NOT;
	}
}

[[nodiscard]] bool Parser::is_bin_op(Token_Type token_type)
{
	return token_type == Token_Type::PLUS ||
		   token_type == Token_Type::MINUS ||
		   token_type == Token_Type::MULTIPLY ||
		   token_type == Token_Type::DIVIDE ||
		   token_type == Token_Type::DIV ||
		   token_type == Token_Type::MOD ||
		   token_type == Token_Type::LESS_THAN ||
		   token_type == Token_Type::GREATER_THAN ||
		   token_type == Token_Type::EQUALS ||
		   token_type == Token_Type::EXCLAIMATION ||
		   token_type == Token_Type::OR ||
		   token_type == Token_Type::AND;
}

[[nodiscard]] bool Parser::is_unary(Token_Type token_type)
{
	return token_type == Token_Type::NOT ||
	       token_type == Token_Type::MINUS;
}

[[nodiscard]] bool Parser::is_stmt(Token_Type token_type)
{
	return token_type == Token_Type::IF ||
		   token_type == Token_Type::OUTPUT ||
		   token_type == Token_Type::CONSTANT ||
		   token_type == Token_Type::SUB_ROUTINE;
}

[[nodiscard]] bool Parser::is_stdlib(const std::string_view name)
{
	return name == "LEN" ||
		   name == "POSITION" ||
		   name == "SUBSTRING" ||
		   name == "STRING_TO_INT" ||
		   name == "STRING_TO_REAL" ||
		   name == "INT_TO_STRING" ||
		   name == "REAL_TO_STRING" ||
		   name == "CHAR_TO_CODE" ||
		   name == "CODE_TO_CHAR" ||
		   name == "RANDOM_INT";
}

[[nodiscard]] bool Parser::is_var_defined(const VarStmt &var_stmt)
{
	std::size_t stop_index{};

	if (!m_var_scope_stack.empty())
	{
		stop_index = m_var_scope_stack.back();
	}

	for (std::size_t i = m_existing_vars.size(); i-- > stop_index;)
	{
		if (m_existing_vars[i].m_name == var_stmt.m_name)
		{
			return true;
		}
	}

	return false;
}

[[nodiscard]] VarStmt Parser::existing_vars_lookup(std::string_view name)
{
	for (auto it{m_existing_vars.rbegin()}; it != m_existing_vars.rend(); it++)
	{
		if (it->m_name == name)
		{
			return *it;
		}
	}

	parse_error_l("couldn't match name with existing variables");
}

[[nodiscard]] FuncDeclStmt &Parser::existing_func_lookup(std::string_view name)
{
	for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++)
	{
		if (it->m_name == name)
		{
			return *it;
		}
	}

	parse_error_l("func doesnt exist");
}

[[nodiscard]] Data_Type Parser::tt_to_dt(Token_Type token_type)
{
	if (token_type == Token_Type::REAL || token_type == Token_Type::REAL_TYPE)
	{
		return Data_Type::REAL;
	}
	else if (token_type == Token_Type::INT || token_type == Token_Type::INT_TYPE)
	{
		return Data_Type::INT;
	}
	else if (token_type == Token_Type::STRING || token_type == Token_Type::STRING_TYPE)
	{
		return Data_Type::STRING;
	}
	parse_error_l("no match type");
}

[[nodiscard]] Data_Type Parser::tt_to_dt(Token token)
{
	if (token.m_type == Token_Type::IDENTIFIER)
	{
		return existing_vars_lookup(token.m_value).m_expr->m_type;
	}
	else
	{
		return tt_to_dt(token.m_type);
	}
}

[[nodiscard]] Operator Parser::tt_to_op(Token_Type token_type)
{
	switch (token_type)
	{
	case (Token_Type::PLUS):
		return Operator::PLUS;
	case (Token_Type::MINUS):
		return Operator::MINUS;
	case (Token_Type::MULTIPLY):
		return Operator::MULTIPLY;
	case (Token_Type::DIVIDE):
		return Operator::DIVIDE;
	case (Token_Type::DIV):
		return Operator::DIV;
	case (Token_Type::MOD):
		return Operator::MOD;
	case (Token_Type::LESS_THAN):
		return Operator::LESS_THAN;
	case (Token_Type::GREATER_THAN):
		return Operator::GREATER_THAN;
	case (Token_Type::EQUALS):
		return Operator::EQUALS;
	case (Token_Type::AND):
		return Operator::AND;
	case (Token_Type::OR):
		return Operator::OR;
	default:
		parse_error_l("no matching operator found");
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
	if (peek().m_type != type)
	{
		parse_error_s("expected `" + to_string(type) + "` got `" + to_string(peek().m_type) + "`");
	}
	else
	{
		return eat();
	}
}
