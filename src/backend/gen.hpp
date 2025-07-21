#ifndef GEN_H
#define GEN_H

#include <fstream>

#include "../frontend/parser.hpp"
#include "../frontend/ast.hpp"

#define gen_error_s(a, b, c) Error{a, b, c, m_source}
#define gen_error_l(a) Error{a}

class Generator {
public:
	Generator() = default;
	explicit Generator(Parser& parser);

	void gen(const std::string& cpp_output_path);

private:
	void gen_stmt(const Stmt& stmt);
	void gen_type(const Data_Type& type);
	void gen_expr(const Expr& expr);
	void gen_atom_expr(const AtomExpr& atom_expr);
	void gen_bin_op_expr(const BinOpExpr& bin_op_expr);

	void type_check(const Data_Type& type1, const Data_Type& type2);
	[[nodiscard]] std::string gen_op(const Operator& op) const;

	[[nodiscard]] bool is_var_defined(const VarStmt& var_stmt);
	[[nodiscard]] std::size_t get_var_index(const VarStmt& var_stmt);
	[[nodiscard]] Data_Type existing_vars_lookup(std::string_view name);
	[[nodiscard]] bool is_func_defined(const FuncCallExpr& func_call_expr);
	[[nodiscard]] std::size_t get_func_index(const FuncCallExpr& func_call_expr);
	[[nodiscard]] Param existing_funcs_lookup(std::string_view name);

	// private members
	AST m_ast{};
	std::vector<VarStmt> m_existing_vars{};
	std::vector<FuncDeclStmt> m_existing_funcs{};
	std::ofstream m_output_file{};
	std::string m_source{};
};

#endif
