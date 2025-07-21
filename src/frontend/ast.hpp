#ifndef AST_HPP
#define AST_HPP

#include <variant>
#include <vector>
#include <string>
#include <memory>

enum class Data_Type {
	DOUBLE,
	STRING,
	NONE
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
	Data_Type m_original_dt{};
	Data_Type m_final_dt{};
	bool m_is_constant{};
	bool m_is_reassignment{};
};

struct Param {
	std::string m_name{};
	Data_Type m_type{};
};

// for matching notation with `Args`
struct Params {
	std::vector<Param> m_params{};
};

struct Args {
	std::vector<Expr> m_exprs{};
};

struct Return {
	std::shared_ptr<Expr> m_return_expr{};
};

struct OutputStmt {
	Args m_args{};
};

struct FuncDeclStmt {
	std::string m_name{};
	Params m_params{};
	std::shared_ptr<Body> m_body{};
	Return m_return{};
	bool is_void{true};
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

struct FuncCallExpr {
	std::string m_name{};
	Args m_args{};
};

struct Expr {
	std::variant<AtomExpr, BinOpExpr, FuncCallExpr> m_expr{};
	Data_Type m_type{};
};

struct AST {
	Body m_body{};
};

#endif
