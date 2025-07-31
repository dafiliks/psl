#ifndef GEN_H
#define GEN_H

#include <fstream>
#include <sstream>
#include <array>

#include "../frontend/parser.hpp"
#include "../frontend/ast.hpp"

#define gen_error_s(a, b, c) \
	Error { a, b, c, m_source }
#define gen_error_l(a) \
	Error { a }

class Generator
{
public:
	Generator() = default;
	explicit Generator(Parser &parser);

	void gen(const std::string &cpp_output_path);

private:
	void gen_stmt(const Stmt &stmt);
	void gen_type(const Data_Type &type);
	void gen_expr(const Expr &expr);
	void gen_atom_expr(const AtomExpr &atom_expr);
	void gen_bin_op_expr(const BinOpExpr &bin_op_expr);
	void gen_op(const Operator &op);

	void gen_params(const Params &params);
	void gen_args(const Args &args, const std::string_view separator);
	void gen_body(const Body &body);

	void type_check(const Data_Type &type1, const Data_Type &type2);
	void type_check_func_args(std::string_view name, const Args &args);
	void op_check(const Data_Type &type1, const Operator &op, const Data_Type &type2);

	void require_lib(const std::string library);
	void change_stream(std::ostringstream &new_stream);

	void check_not_constant_reassignment(const VarStmt &var_stmt);
	void check_reassignment_same_type(const VarStmt &var_stmt);
	void check_capital_name(const VarStmt &var_stmt);

	void check_func_defined(const std::string_view name);
	void check_func_not_redefined(const std::string_view name);
	void check_func_non_void(const std::string_view name);
	void check_arg_length_matches(const std::string_view name, const Args &args);

	void check_expr_is_type(const Expr &expr, Data_Type data_type);
	void check_expr_is_not_str(const Expr &expr);

	[[nodiscard]] Param &existing_param_lookup(std::string_view name);
	[[nodiscard]] FuncDeclStmt &existing_func_lookup(std::string_view name);

	[[nodiscard]] bool is_lib_loaded(std::string_view library);

	// private members
	AST m_ast{};
	std::vector<VarStmt> m_existing_vars{};
	std::vector<FuncDeclStmt> m_existing_funcs{};

	[[maybe_unused]] constexpr static std::array<std::string_view, 9> m_reserved_func_names{
		"LEN",
		"POSITION",
		"SUBSTRING",
		"STRING_TO_INT",
		"STRING_TO_REAL",
		"INT_TO_STRING",
		"REAL_TO_STRING",
		"CHAR_TO_CODE",
		"CODE_TO_CHAR",
	};

	std::size_t m_random_int_count{};

	std::ofstream m_output_file{};
	std::string m_source{};

	std::ostringstream m_lib_stream{};
	std::ostringstream m_record_stream{};
	std::ostringstream m_func_stream{};
	std::ostringstream m_main_stream{};

	std::ostringstream *m_current_stream{};

	std::vector<std::string> m_used_libs{};
};

#endif
