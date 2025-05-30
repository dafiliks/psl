#ifndef AST_HPP
#define AST_HPP

#include <variant>
#include <string>
#include <memory>

struct Expr;

struct VarStmt {
	std::string m_name{};
	std::unique_ptr<Expr> m_expr{};
};

struct Stmt {
	std::variant<VarStmt> m_stmt{};
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

struct AtomExpr {
	std::variant<IntExpr, FloatExpr, StrExpr> m_atom{};
};

struct BinOpExpr {
	std::unique_ptr<Expr> m_lhs{};
	std::unique_ptr<Expr> m_rhs{};
	Token_Type m_op{};
};

struct Expr {
	std::variant<AtomExpr, BinOpExpr> m_expr{};
};

struct Program {
	std::vector<Stmt> m_body{};
};

#endif
