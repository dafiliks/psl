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

	void gen();

private:
	void gen_stmt(const Stmt& stmt);
	void gen_expr(const Expr& expr);
	void gen_atom_expr(const AtomExpr& atom_expr);
	void gen_bin_op_expr(const BinOpExpr& bin_op_expr);

	void type_check(const Data_Type& type1, const Data_Type& type2);
	[[nodiscard]] std::string gen_op(const Operator& op) const;

	// private members
	AST m_ast{};
	std::ofstream m_output_file{};
	std::string m_source{};
};

#endif
