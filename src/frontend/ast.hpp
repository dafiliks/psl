#ifndef AST_HPP
#define AST_HPP

#include <variant>
#include <vector>
#include <string>
#include <memory>

enum class Data_Type {
	DOUBLE,
	STRING
};

enum class Operator {
	PLUS,
	MINUS,
	MULTIPLY,
	DIVIDE,
	DIV,
	MOD
};

struct Expr; struct StrExpr; struct VarExpr; struct Scope; struct Body;

struct VarStmt {
	std::string m_name{};
	std::shared_ptr<Expr> m_expr{};
	bool m_is_constant{};
	bool m_is_reassignment{};
};

struct OutputStmt {
	std::vector<Expr> m_args{};
};

struct Args {
	std::vector<VarExpr> m_var_exprs{};
};

struct FuncDeclStmt {
	std::string m_name{};
	Args m_args{};
	std::shared_ptr<Body> m_body{};
};

struct Stmt {
	std::variant<VarStmt, OutputStmt, FuncDeclStmt> m_stmt{};
};

struct Body {
	std::vector<Stmt> m_stmts{};
};

struct IntExpr {
	int m_value{};
};

struct FloatExpr {
	float m_value{};
};

struct StrExpr {
	std::string m_value{};
};

struct VarExpr {
	std::string m_name{};
};

struct UserInputExpr {};

struct AtomExpr {
	std::variant<IntExpr, FloatExpr, StrExpr, VarExpr, UserInputExpr> m_atom{};
};

struct BinOpExpr {
	std::shared_ptr<Expr> m_lhs{};
	std::shared_ptr<Expr> m_rhs{};
	Operator m_op{};
};

struct Expr {
	std::variant<AtomExpr, BinOpExpr> m_expr{};
	Data_Type m_type{};
};

struct AST {
	Body m_body{};
};

#endif
