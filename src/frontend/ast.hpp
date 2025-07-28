#ifndef AST_HPP
#define AST_HPP

#include <variant>
#include <vector>
#include <string>
#include <memory>

enum class Data_Type
{
	INT,
	STRING,
	REAL,
	CHAR,
	NONE,
};

enum class Operator
{
	PLUS,
	MINUS,
	MULTIPLY,
	DIVIDE,
	DIV,
	MOD,
	LESS_THAN,
	GREATER_THAN,
	EQUALS,
	NOT_EQUALS,
	LESS_THAN_OET,
	GREATER_THAN_OET,
	AND,
	OR,
};

struct Expr;
struct StrExpr;
struct VarExpr;
struct Scope;
struct Body;
struct RelationalOpExpr;
struct AtomExpr;

struct VarStmt
{
	std::string m_name{};
	std::shared_ptr<Expr> m_expr{};
	std::shared_ptr<Expr> m_previous_expr{};
	bool m_is_constant{};
	bool m_is_reassignment{};
};

struct Param
{
	std::string m_name{};
	Data_Type m_type{};
};

struct Params
{
	std::vector<Param> m_params{};
};

struct Args
{
	std::vector<Expr> m_exprs{};
};

struct Return
{
	std::shared_ptr<Expr> m_return_expr{};
};

struct OutputStmt
{
	Args m_args{};
};

struct FuncDeclStmt
{
	std::string m_name{};
	Params m_params{};
	std::size_t m_token_index_start{};
	std::shared_ptr<Body> m_body{};
	Return m_return{};
	bool m_is_void{true};
	bool m_is_called{false};
};

struct FuncCallStmt
{
	std::string m_name{};
	Args m_args{};
};

// use inheritance later on
struct RepeatUntilStmt
{
	std::shared_ptr<Expr> m_condition_expr{};
	std::shared_ptr<Body> m_body{};
};

struct WhileStmt
{
	std::shared_ptr<Expr> m_condition_expr{};
	std::shared_ptr<Body> m_body{};
};

struct IfStmt
{
	std::shared_ptr<Expr> m_condition_expr{};
	std::shared_ptr<Body> m_body{};
};

struct ElseIfStmt
{
	std::shared_ptr<Expr> m_condition_expr{};
	std::shared_ptr<Body> m_body{};
};

struct ElseStmt
{
	std::shared_ptr<Body> m_body{};
};

struct ForToStmt
{
	VarStmt m_var_stmt{};
	std::shared_ptr<Expr> m_boundary{};
	std::shared_ptr<Expr> m_step{};
	std::shared_ptr<Body> m_body{};
};

struct Stmt
{
	std::variant<VarStmt, OutputStmt, FuncDeclStmt, FuncCallStmt,
				 RepeatUntilStmt, WhileStmt, IfStmt, ElseIfStmt, ElseStmt,
				 ForToStmt>
		m_stmt{};
};

struct Body
{
	std::vector<Stmt> m_stmts{};
};

struct IntExpr
{
	int m_value{};
};

struct RealExpr
{
	double m_value{};
};

struct StrExpr
{
	std::string m_value{};
};

struct VarExpr
{
	std::string m_name{};
};

struct UserInputExpr
{
};

struct FuncCallExpr
{
	std::string m_name{};
	Args m_args{};
};

struct LenCallExpr
{
	std::shared_ptr<Expr> m_str_expr{};
};

struct PositionCallExpr
{
	std::shared_ptr<Expr> m_str_expr{};
	std::shared_ptr<Expr> m_char_expr{};
};

struct SubStrCallExpr
{
	std::shared_ptr<Expr> m_num1_expr{};
	std::shared_ptr<Expr> m_num2_expr{};
	std::shared_ptr<Expr> m_str_expr{};
};

struct StrToIntCallExpr
{
	std::shared_ptr<Expr> m_str_expr{};
};

struct StrToRealCallExpr
{
	std::shared_ptr<Expr> m_str_expr{};
};

struct IntToStrCallExpr
{
	std::shared_ptr<Expr> m_int_expr{};
};

struct RealToStrCallExpr
{
	std::shared_ptr<Expr> m_real_expr{};
};

struct CharToCodeCallExpr
{
	std::shared_ptr<Expr> m_char_expr{};
};

struct CodeToCharCallExpr
{
	std::shared_ptr<Expr> m_int_expr{};
};

struct RandomIntCallExpr
{
	std::shared_ptr<Expr> m_int1_expr{};
	std::shared_ptr<Expr> m_int2_expr{};
};

struct AtomExpr
{
	std::variant<IntExpr, RealExpr, StrExpr, VarExpr,
				 UserInputExpr, FuncCallExpr, LenCallExpr, PositionCallExpr,
				 SubStrCallExpr, StrToIntCallExpr, StrToRealCallExpr, IntToStrCallExpr,
				 RealToStrCallExpr, CharToCodeCallExpr, CodeToCharCallExpr, RandomIntCallExpr>
		m_atom{};
};

struct BinOpExpr
{
	std::shared_ptr<Expr> m_lhs{};
	std::shared_ptr<Expr> m_rhs{};
	Operator m_op{};
};

struct UnaryOpExpr
{
	std::shared_ptr<Expr> m_unary_expr{};
	Operator m_op{};
};

struct Expr
{
	std::variant<AtomExpr, BinOpExpr, UnaryOpExpr> m_expr{};
	Data_Type m_type{};
};

struct AST
{
	Body m_body{};
};

#endif
