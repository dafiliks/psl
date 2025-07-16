#ifndef AST_HPP
#define AST_HPP

#include <variant>
#include <vector>
#include <string>
#include <memory>

// types
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

struct Expr;

struct StrExpr;

struct VarExpr;

struct Scope;

struct VarStmt {
	std::string m_name{};
	std::unique_ptr<Expr> m_expr{};
	bool m_is_constant{};
    bool m_is_reassignment{};
};

struct OutputStmt {
    std::vector<Expr> m_args{};
};

struct FuncDeclStmt {
    std::string m_name{};
    std::vector<VarExpr> m_args{};
    std::unique_ptr<Scope> m_scope{};
};

struct Stmt {
	std::variant<VarStmt, OutputStmt, FuncDeclStmt> m_stmt{};
};

struct Scope {
    std::vector<Stmt> m_body{};
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
	std::unique_ptr<Expr> m_lhs{};
	std::unique_ptr<Expr> m_rhs{};
	Operator m_op{};
};

struct Expr {
	std::variant<AtomExpr, BinOpExpr> m_expr{};
	Data_Type m_type{};
};

struct Program {
    Scope m_scope{};
};

#endif
