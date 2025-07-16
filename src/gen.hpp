#ifndef GEN_H
#define GEN_H

#include <fstream>
#include "parser.hpp"
#include "ast.hpp"

class Generator {
public:
	Generator(Parser&& parser);
	~Generator() = default;

	void gen();
	void gen_stmt(Stmt& stmt);
	void gen_expr(Expr& expr);
	void gen_atom_expr(AtomExpr atom_expr);
	void gen_bin_op_expr(BinOpExpr& bin_op_expr);
    void type_check(Data_Type type1, Data_Type type2);
private:
    Error m_gen_error{};
	Program m_program{};
	std::ofstream m_output_file{};
};

#endif
