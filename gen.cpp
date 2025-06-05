#include <fstream>
#include <cctype>
#include <algorithm>
#include <iostream>
#include <variant>
#include "gen.hpp"
#include "ast.hpp"
#include "error.hpp"

Generator::Generator(Parser&& parser)
: m_program(std::move(parser.get_program())),
  m_gen_error(parser.get_parse_error()),
  m_output_file(m_gen_error.m_file_name.substr(0, m_gen_error.m_file_name.find(".")) + ".cpp") {}

void Generator::gen()
{
	m_output_file << "#include <iostream>\n";
    m_output_file << "#include <string>\n\n";
	m_output_file << "int main()\n";
	m_output_file << "{\n";
	for (auto& stmt : m_program.m_body) {
		gen_stmt(stmt);
	}
	m_output_file << "	return 0;\n";
	m_output_file << "}\n";
	m_output_file.close();
}

// TODO: visit with lambdas in the future
void Generator::gen_stmt(Stmt& stmt)
{
	struct StmtVisitor {
		Generator& gen;
		void operator()(VarStmt& var_stmt)
		{
			gen.m_output_file << "	";

			if (var_stmt.m_is_constant) {
				if (!std::all_of(var_stmt.m_name.begin(),
				                  var_stmt.m_name.end(),
				                  [](char c) { return isupper(c); } ))
					std::cerr << "CONSTANTS SHOULD ALWAYS HAVE ALL CAPS NAMES";

				gen.m_output_file << "const ";
			}

			if (var_stmt.m_expr->m_type == Token_Type::STRING_LIT) {
				gen.m_output_file << "std::string " << var_stmt.m_name << " = ";
			} else if (var_stmt.m_expr->m_type == Token_Type::INT_LIT) {
				gen.m_output_file << "int " << var_stmt.m_name << " = ";
			} else if (var_stmt.m_expr->m_type == Token_Type::FLOAT) {
				gen.m_output_file << "float " << var_stmt.m_name << " = ";
			}
			gen.gen_expr(*var_stmt.m_expr);
			gen.m_output_file << ";\n";
		}
	};

	StmtVisitor stmt_visitor{*this};
	std::visit(stmt_visitor, stmt.m_stmt);
}

void Generator::gen_expr(Expr& expr)
{
	struct ExprVisitor {
		Generator& gen;
		void operator()(AtomExpr atom_expr)
		{
			gen.gen_atom_expr(atom_expr);
		}
		void operator()(BinOpExpr& bin_op_expr)
		{
            gen.type_check(bin_op_expr.m_lhs->m_type, bin_op_expr.m_rhs->m_type);
			gen.gen_expr(*bin_op_expr.m_lhs);
			gen.m_output_file << to_string(bin_op_expr.m_op);
			gen.gen_expr(*bin_op_expr.m_rhs);
		}
	};

	ExprVisitor expr_visitor{*this};
	std::visit(expr_visitor, expr.m_expr);
}

void Generator::gen_atom_expr(AtomExpr atom_expr)
{
	struct AtomExprVisitor {
		Generator& gen;
		void operator()(StrExpr str_expr)
		{
			gen.m_output_file << "std::string(" << str_expr.m_value << ")";
		}
		void operator()(IntExpr int_expr)
		{
			gen.m_output_file << int_expr.m_value;
		}
		void operator()(FloatExpr float_expr)
		{
			gen.m_output_file << float_expr.m_value;
			gen.m_output_file << "f";
		}
	};

	AtomExprVisitor atom_expr_visitor{*this};
	std::visit(atom_expr_visitor, atom_expr.m_atom);
}

void Generator::type_check(Token_Type type1, Token_Type type2)
{
    if (type1 != type2) {
        m_gen_error.error("Type mismatch");
    }
}
