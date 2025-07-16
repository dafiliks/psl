#include <cassert>
#include <iostream>
#include <variant>
#include <memory>
#include "parser.hpp"
#include "lexer.hpp"
#include "error.hpp"
#include "ast.hpp"

Parser::Parser(Lexer lexer)
: m_tokens(lexer.get_tokens()), m_parse_error(lexer.get_lex_error()) {}

void Parser::parse()
{
    for (auto& i : m_tokens) {
        std::cout << "TOKEN: " << i.m_value << " | TYPE: " << to_string(i.m_type) << "\n";
    }

    std::cout << "\n";

	while (peek().m_type != Token_Type::END_OF_FILE) {
		m_program.m_scope.m_body.push_back(parse_stmt());
	}
}

Stmt Parser::parse_stmt()
{
	if (peek().m_type == Token_Type::IDENTIFIER ||
	    peek().m_type == Token_Type::CONSTANT) {
		return Stmt{parse_vd_or_assignment()};
	} else if (peek().m_type == Token_Type::OUTPUT) {
        return Stmt{parse_output_stmt()};
    } else if (peek().m_type == Token_Type::SUB_ROUTINE) {
        return Stmt{parse_func_decl_stmt()};
    } else {
		eat();
	}

    return Stmt{};
}


FuncDeclStmt Parser::parse_func_decl_stmt()
{
    FuncDeclStmt func_decl_stmt{};

    eat();

    func_decl_stmt.m_name = try_eat(Token_Type::IDENTIFIER).m_value;

    try_eat(Token_Type::O_PAREN);

    while (peek().m_type != Token_Type::C_PAREN && peek().m_type != Token_Type::END_OF_FILE) {
        if (peek().m_type == Token_Type::COMMA) {
            eat();
        }
        else if (peek().m_type == Token_Type::IDENTIFIER) {
            func_decl_stmt.m_args.push_back(VarExpr{peek().m_value});
            eat();
        }
    }

    try_eat(Token_Type::C_PAREN);

    func_decl_stmt.m_scope = std::make_unique<Scope>();

    while (peek().m_type != Token_Type::END_SUB_ROUTINE && peek().m_type != Token_Type::END_OF_FILE) {
        func_decl_stmt.m_scope->m_body.push_back(parse_stmt());
    }

    try_eat(Token_Type::END_SUB_ROUTINE);

    return func_decl_stmt;
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
	try_eat(Token_Type::MINUS);

    auto got = existing_vars_type.find(var_stmt.m_name);
    if (got != existing_vars_type.end()) {

        auto got_const = existing_const_vars.find(var_stmt.m_name);
        if (got_const != existing_const_vars.end()) {
            m_parse_error.error("can't reassign a constant variable");
        }

        var_stmt.m_is_reassignment = true;

        Data_Type type{tt_to_dt(peek().m_type)};
        if (peek().m_type == Token_Type::IDENTIFIER) {
            type = existing_vars_lookup(peek().m_value);
        }

        if (existing_vars_lookup(var_stmt.m_name) != type) {
            m_parse_error.error("can't assign different type to previously defined variable");
        }
    }

	var_stmt.m_expr = std::make_unique<Expr>(parse_expr());

    existing_vars_type.insert({var_stmt.m_name, var_stmt.m_expr->m_type});

    if (var_stmt.m_is_constant) {
        existing_const_vars.insert({var_stmt.m_name, var_stmt.m_expr->m_type});
    }

	return var_stmt;
}

OutputStmt Parser::parse_output_stmt()
{
    OutputStmt output_stmt{};

    do {
        eat();
        output_stmt.m_args.push_back(parse_expr());
    } while (peek().m_type == Token_Type::COMMA);

    return output_stmt;
}

Data_Type Parser::existing_vars_lookup(std::string name)
{
    auto got = existing_vars_type.find(name);
    if (got != existing_vars_type.end()) {
        return got->second;
    } else {
        m_parse_error.error("variable undeclared");
    }
}

Expr Parser::parse_expr()
{
	Expr expr{};

    if (peek().m_type == Token_Type::FLOAT) {
        expr.m_type = Data_Type::DOUBLE;
    } else if (peek().m_type == Token_Type::STRING_LIT) {
        expr.m_type = Data_Type::STRING;
    } else if (peek().m_type == Token_Type::IDENTIFIER) {
        expr.m_type = existing_vars_lookup(peek().m_value);
    } else if (peek().m_type == Token_Type::USER_INPUT) {
        // suppose a <- USERINPUT
        // in this case, we treat `a` as a string
        expr.m_type = Data_Type::STRING;
    }

	if (is_bin_op(peek(1).m_type)) {
		expr.m_expr = parse_bin_op_expr();
		return expr;
	}

	expr.m_expr = parse_atom();

    eat();

	return expr;
}

IntExpr Parser::parse_int_expr()
{
	return IntExpr{std::stoi(peek().m_value)};
}

FloatExpr Parser::parse_float_expr()
{
	return FloatExpr{std::stof(peek().m_value)};
}

StrExpr Parser::parse_str_expr()
{
	return StrExpr{peek().m_value};
}

VarExpr Parser::parse_var_expr()
{
    return VarExpr{peek().m_value};
}

UserInputExpr Parser::parse_ui_expr()
{
    return UserInputExpr{};
}

AtomExpr Parser::parse_atom()
{
	switch (peek().m_type) {
	case Token_Type::FLOAT:
		return AtomExpr{parse_float_expr()};
		break;
	case Token_Type::STRING_LIT:
		return AtomExpr{parse_str_expr()};
		break;
    case Token_Type::IDENTIFIER:
        return AtomExpr{parse_var_expr()};
        break;
    case Token_Type::USER_INPUT:
        return AtomExpr{parse_ui_expr()};
        break;
    default:
        return AtomExpr{};
	}
}

std::unique_ptr<Expr> Parser::replace_with_type_if_identifier(Data_Type type, std::string value, bool is_rhs_binop) {
    // kinda ugly, if rhs is a binop, parse it as a binop recursively
    if (peek().m_type == Token_Type::IDENTIFIER) {
        if (is_rhs_binop)
            return std::make_unique<Expr>(Expr{parse_bin_op_expr(), existing_vars_lookup(value)});
	    return std::make_unique<Expr>(Expr{parse_atom(), existing_vars_lookup(value)});
    } else {
        if (is_rhs_binop)
            return std::make_unique<Expr>(Expr{parse_bin_op_expr(), type});
        return std::make_unique<Expr>(Expr{parse_atom(), type});
    }
}

BinOpExpr Parser::parse_bin_op_expr()
{
    BinOpExpr bin_op_expr{};

    bin_op_expr.m_lhs = replace_with_type_if_identifier(tt_to_dt(peek().m_type), peek().m_value, false);
    eat();

	bin_op_expr.m_op = tt_to_op(eat().m_type);

	if (is_bin_op(peek(1).m_type)) {
        // is a rhs binop expr, hence the `true` argument
        bin_op_expr.m_rhs = replace_with_type_if_identifier(tt_to_dt(peek().m_type), peek().m_value, true);
	} else {
        bin_op_expr.m_rhs = replace_with_type_if_identifier(tt_to_dt(peek().m_type), peek().m_value, false);
        eat();
    }

	return bin_op_expr;
}

bool Parser::is_bin_op(Token_Type type)
{
	return type == Token_Type::PLUS     ||
	       type == Token_Type::MINUS    ||
	       type == Token_Type::MULTIPLY ||
	       type == Token_Type::DIVIDE   ||
           type == Token_Type::DIV      ||
           type == Token_Type::MOD;
}

Data_Type Parser::tt_to_dt(Token_Type tt) {
    if (tt == Token_Type::FLOAT) {
        return Data_Type::DOUBLE;
    } else if (tt == Token_Type::STRING_LIT) {
        return Data_Type::STRING;
    }
    return Data_Type::STRING;
}

Operator Parser::tt_to_op(Token_Type tt) {
    if (tt == Token_Type::PLUS) {
        return Operator::PLUS;
    } else if (tt == Token_Type::MINUS) {
        return Operator::MINUS;
    } else if (tt == Token_Type::MULTIPLY) {
        return Operator::MULTIPLY;
    } else if (tt == Token_Type::DIVIDE) {
        return Operator::DIVIDE;
    } else if (tt == Token_Type::DIV) {
        return Operator::DIV;
    } else if (tt == Token_Type::MOD) {
        return Operator::MOD;
    }
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
