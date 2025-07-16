#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include "lexer.hpp"
#include "ast.hpp"

static std::unordered_map<std::string, Data_Type> existing_vars_type{};
static std::unordered_map<std::string, Data_Type> existing_const_vars{};

class Parser {
public:
	Parser(Lexer lexer);
	~Parser() = default;

	void parse();
	Stmt parse_stmt();
    FuncDeclStmt parse_func_decl_stmt();
	VarStmt parse_vd_or_assignment();
    OutputStmt parse_output_stmt();
	Expr parse_expr();
	IntExpr parse_int_expr();
	FloatExpr parse_float_expr();
	StrExpr parse_str_expr();
    VarExpr parse_var_expr();
    UserInputExpr parse_ui_expr();
    std::unique_ptr<Expr> replace_with_type_if_identifier(Data_Type type, std::string value, bool is_rhs_binop);
	AtomExpr parse_atom();
	BinOpExpr parse_bin_op_expr();
    Data_Type existing_vars_lookup(std::string name);
	bool is_bin_op(Token_Type type);
    Data_Type tt_to_dt(Token_Type tt);
    Operator tt_to_op(Token_Type tt);
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
