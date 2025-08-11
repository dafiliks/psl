#ifndef AST_HPP
#define AST_HPP

#include <variant>
#include <vector>
#include <string>
#include <memory>

enum class DataType {
	UNRESOLVED,
	INT,
	STRING,
	REAL,
	CHAR,
	NONE,
	USER_DEFINED_TYPE,
};

enum class Operator {
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
	NOT,
};

struct Expr;
struct StrExpr;
struct VarExpr;
struct Scope;
struct Body;
struct RelationalOpExpr;
struct AtomExpr;

struct VarStmt {
	std::string m_name{};
	std::unique_ptr<Expr> m_expr{};

	[[maybe_unused]] std::unique_ptr<Expr> m_previous_expr{};

	bool m_is_constant{};
	bool m_is_reassignment{};

	[[maybe_unused]] bool m_is_1d_list{};
	[[maybe_unused]] bool m_is_2d_list{};
	[[maybe_unused]] std::string m_record_name{};
};


struct Param {
	std::string m_name{};
	DataType m_type{};
};

struct Params {
	std::vector<Param> m_params{};
};

struct Args {
	std::vector<Expr> m_exprs{};
};

struct Return {
	std::unique_ptr<Expr> m_return_expr{};
};

struct OutputStmt {
	Args m_args{};
};

struct FuncDeclStmt {
	std::string m_name{};
	Params m_params{};
	std::size_t m_token_index_start{};
	std::unique_ptr<Body> m_body{};
	Return m_return{};
	bool m_is_void{true};
	bool m_is_called{false};
};

struct FuncCallStmt {
	std::string m_name{};
	Args m_args{};
};

struct RepeatUntilStmt {
	std::unique_ptr<Expr> m_condition_expr{};
	std::unique_ptr<Body> m_body{};
};

struct WhileStmt {
	std::unique_ptr<Expr> m_condition_expr{};
	std::unique_ptr<Body> m_body{};
};

struct IfStmt {
	std::unique_ptr<Expr> m_condition_expr{};
	std::unique_ptr<Body> m_body{};
};

struct ElseIfStmt {
	std::unique_ptr<Expr> m_condition_expr{};
	std::unique_ptr<Body> m_body{};
};

struct ElseStmt {
	std::unique_ptr<Body> m_body{};
};

struct ForToStmt {
	VarStmt m_var_stmt{};
	std::unique_ptr<Expr> m_boundary{};
	std::unique_ptr<Expr> m_step{};
	std::unique_ptr<Body> m_body{};
};

struct ForInStmt {
	std::unique_ptr<VarStmt> m_declaration{};
	std::unique_ptr<Expr> m_range{};
	std::unique_ptr<Body> m_body{};
};

struct ListAccessStmt {
	std::string m_name{};
	std::unique_ptr<Expr> m_row{};
	std::unique_ptr<Expr> m_col{};
	std::unique_ptr<Expr> m_expr{};
};

struct FieldStmt {
	std::string m_name{};
	DataType m_type{};
};

struct Fields
{
	std::vector<FieldStmt> m_fields{};
};

struct RecordStmt {
	std::string m_name{};
	Fields m_fields{};
};

struct FieldAccessStmt {
	std::string m_name{};
	std::string m_field_name{};
	std::unique_ptr<Expr> m_expr{};
};

struct Stmt {
	std::variant<VarStmt, OutputStmt, FuncDeclStmt, FuncCallStmt,
	             RepeatUntilStmt, WhileStmt, IfStmt, ElseIfStmt, ElseStmt,
	             ForToStmt, ForInStmt, ListAccessStmt, FieldStmt, RecordStmt,
	             FieldAccessStmt> m_stmt{};
};

struct Body {
	std::vector<Stmt> m_stmts{};
};

struct IntExpr {
	int m_value{};
};

struct RealExpr {
	double m_value{};
};

struct StrExpr {
	std::string m_value{};
};

struct VarExpr {
	std::string m_name{};
};

struct UserInputExpr {
};

struct FuncCallExpr {
	std::string m_name{};
	Args m_args{};
};

struct LenCallExpr {
	std::unique_ptr<Expr> m_expr{};
};

struct PositionCallExpr {
	std::unique_ptr<Expr> m_str_expr{};
	std::unique_ptr<Expr> m_char_expr{};
};

struct SubStrCallExpr {
	std::unique_ptr<Expr> m_num1_expr{};
	std::unique_ptr<Expr> m_num2_expr{};
	std::unique_ptr<Expr> m_str_expr{};
};

struct StrToIntCallExpr {
	std::unique_ptr<Expr> m_str_expr{};
};

struct StrToRealCallExpr {
	std::unique_ptr<Expr> m_str_expr{};
};

struct IntToStrCallExpr {
	std::unique_ptr<Expr> m_int_expr{};
};

struct RealToStrCallExpr {
	std::unique_ptr<Expr> m_real_expr{};
};

struct CharToCodeCallExpr {
	std::unique_ptr<Expr> m_char_expr{};
};

struct CodeToCharCallExpr {
	std::unique_ptr<Expr> m_int_expr{};
};

struct RandomIntCallExpr {
	std::unique_ptr<Expr> m_int1_expr{};
	std::unique_ptr<Expr> m_int2_expr{};
};

struct ListAccessExpr {
	std::string m_name{};
	std::unique_ptr<Expr> m_row{};
	std::unique_ptr<Expr> m_col{};
};

struct FieldAccessExpr {
	std::string m_name{};
	std::string m_field_name{};
};

struct ObjectCreationExpr {
	std::string m_record_name{};
	Args m_args{};
};

struct AtomExpr {
	std::variant<IntExpr, RealExpr, StrExpr, VarExpr,
	             UserInputExpr, FuncCallExpr, LenCallExpr, PositionCallExpr,
	             SubStrCallExpr, StrToIntCallExpr, StrToRealCallExpr, IntToStrCallExpr,
	             RealToStrCallExpr, CharToCodeCallExpr, CodeToCharCallExpr, RandomIntCallExpr,
	             ListAccessExpr, FieldAccessExpr, ObjectCreationExpr> m_atom{};
};

struct BinOpExpr {
	std::unique_ptr<Expr> m_lhs{};
	std::unique_ptr<Expr> m_rhs{};
	Operator m_op{};
};

struct UnaryOpExpr {
	std::unique_ptr<Expr> m_unary_expr{};
	Operator m_op{};
};

struct ParenExpr {
	std::unique_ptr<Expr> m_expr{};
};

struct ListExpr {
	std::vector<Expr> m_exprs{};
};

struct Expr {
	std::variant<AtomExpr, BinOpExpr, UnaryOpExpr, ParenExpr, ListExpr> m_expr{};
	DataType m_type{};
};

struct AST {
	Body m_body{};
};

#endif
