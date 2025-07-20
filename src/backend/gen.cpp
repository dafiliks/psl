#include <fstream>
#include <cctype>
#include <algorithm>
#include <iostream>
#include <variant>

#include "gen.hpp"
#include "../frontend/ast.hpp"
#include "../utils/error.hpp"
#include "../frontend/parser.hpp"

Generator::Generator(Parser& parser) : m_ast(parser.get_ast()), m_source(parser.get_source()) {}

void Generator::gen()
{
	m_output_file << "#include <iostream>\n";
	m_output_file << "#include <string>\n";
	m_output_file << "#include <cmath>\n";
	m_output_file << "int main()\n";
	m_output_file << "{\n";

	for (auto& stmt : m_ast.m_body.m_stmts) {
		gen_stmt(stmt);
		m_output_file << ";\n";
	}

	m_output_file << "	return 0;\n";
	m_output_file << "}\n";

	m_output_file.close();
}

void Generator::gen_stmt(const Stmt& stmt)
{
	struct StmtVisitor {
		Generator& gen;

		void operator()(const VarStmt& var_stmt)
		{
			gen.m_output_file << "	";

			if (var_stmt.m_is_reassignment) {
				gen.m_output_file << var_stmt.m_name << " = ";
			} else {
				if (var_stmt.m_is_constant) {
					gen.m_output_file << "const ";
				}

				if (var_stmt.m_expr->m_type == Data_Type::STRING) {
					gen.m_output_file << "std::string " << var_stmt.m_name << " = ";
				} else if (var_stmt.m_expr->m_type == Data_Type::DOUBLE) {
					gen.m_output_file << "double " << var_stmt.m_name << " = ";
				}
			}

			gen.gen_expr(*var_stmt.m_expr);
		}

		void operator()(const OutputStmt& output_stmt)
		{
			gen.m_output_file << "	std::cout << ";

			for (std::size_t i = 0; i < output_stmt.m_args.size(); i++) {
				gen.gen_expr(output_stmt.m_args[i]);
				if (i < output_stmt.m_args.size() - 1)
					gen.m_output_file << " << ";
			}
		}

		void operator()(const FuncDeclStmt& func_decl_stmt)
		{
			// SOON (TM)
		}
	};

	StmtVisitor stmt_visitor{*this};
	std::visit(stmt_visitor, stmt.m_stmt);
}

void Generator::gen_expr(const Expr& expr)
{
	struct ExprVisitor {
		Generator& gen;

		void operator()(const AtomExpr& atom_expr)
		{
			gen.gen_atom_expr(atom_expr);
		}

		void operator()(const BinOpExpr& bin_op_expr)
		{
			const Data_Type& type1{bin_op_expr.m_lhs->m_type};
			const Data_Type& type2{bin_op_expr.m_rhs->m_type};

			gen.type_check(type1, type2);

			if (type1 == Data_Type::STRING && type2 == Data_Type::STRING) {
				if (bin_op_expr.m_op != Operator::PLUS) {
					gen_error_l("only concatenation can be performed between strings");
				}
			}

			gen.gen_expr(*bin_op_expr.m_lhs);

			gen.m_output_file << gen.gen_op(bin_op_expr.m_op);

			gen.gen_expr(*bin_op_expr.m_rhs);
		}
	};

	ExprVisitor expr_visitor{*this};
	std::visit(expr_visitor, expr.m_expr);
}

void Generator::gen_atom_expr(const AtomExpr& atom_expr)
{
	struct AtomExprVisitor {
		Generator& gen;

		void operator()(const StrExpr& str_expr)
		{
			gen.m_output_file << "std::string(" << str_expr.m_value << ")";
		}

		void operator()(const IntExpr& int_expr)
		{
			gen.m_output_file << int_expr.m_value;
		}

		void operator()(const FloatExpr& float_expr)
		{
			gen.m_output_file << float_expr.m_value;
		}

		void operator()(const VarExpr& var_expr)
		{
			gen.m_output_file << var_expr.m_name;
		}

		void operator()(const UserInputExpr& user_input_expr)
		{
			gen.m_output_file << "[](){ std::string s; std::getline(std::cin, s); return s; }()";
		}
	};

	AtomExprVisitor atom_expr_visitor{*this};
	std::visit(atom_expr_visitor, atom_expr.m_atom);
}

void Generator::type_check(const Data_Type& type1, const Data_Type& type2)
{
	if (type1 != type2) {
		gen_error_l("type mismatch");
	}
}

[[nodiscard]] std::string Generator::gen_op(const Operator& op) const
{
	switch (op) {
	case (Operator::PLUS):     return "+";
	case (Operator::MINUS):    return "-";
	case (Operator::MULTIPLY): return "*";
	case (Operator::DIVIDE):   return "/";
	case (Operator::DIV):      return "DIV";
	case (Operator::MOD):      return "MOD";
	}
}
