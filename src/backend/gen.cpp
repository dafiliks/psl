#include <fstream>
#include <cctype>
#include <algorithm>
#include <iostream>
#include <variant>

#include "gen.hpp"
#include "../frontend/ast.hpp"
#include "../utils/error.hpp"
#include "../frontend/parser.hpp"

Generator::Generator(Parser& parser) : m_ast(parser.get_ast()), m_source(parser.get_source()),
                                       m_existing_vars(parser.get_existing_vars()), m_existing_funcs(parser.get_existing_funcs()) {}

void Generator::gen(const std::string& cpp_output_path)
{
	m_output_file.open(cpp_output_path);

	m_output_file << "#include <iostream>\n";
	m_output_file << "#include <string>\n";
	m_output_file << "#include <any>\n";
	m_output_file << "#include <cmath>\n";
	m_output_file << "int main()\n";
	m_output_file << "{\n";

	for (auto& stmt : m_ast.m_body.m_stmts) {
		gen_stmt(stmt);
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
				} else if (var_stmt.m_expr->m_type == Data_Type::NONE) {
					gen.m_output_file << "auto " << var_stmt.m_name << " = ";
				}
			}

			gen.gen_expr(*var_stmt.m_expr);

			gen.m_output_file << ";\n";
		}

		void operator()(const OutputStmt& output_stmt)
		{
			gen.m_output_file << "	std::cout << ";

			for (std::size_t i = 0; i < output_stmt.m_args.m_exprs.size(); i++) {
				gen.gen_expr(output_stmt.m_args.m_exprs[i]);
				if (i < output_stmt.m_args.m_exprs.size() - 1)
					gen.m_output_file << " << ";
			}

			gen.m_output_file << ";\n";
		}

		void operator()(const FuncDeclStmt& func_decl_stmt)
		{
			if (func_decl_stmt.is_void) {
				gen.m_output_file << "void ";
			} else {
				gen.m_output_file << "auto ";
			}

			gen.m_output_file << func_decl_stmt.m_name;

			gen.m_output_file << "(";

			for (std::size_t i{0}; i < func_decl_stmt.m_params.m_params.size(); i++) {
				gen.gen_type(gen.existing_funcs_lookup(func_decl_stmt.m_params.m_params[i].m_name).m_type);

				gen.m_output_file << func_decl_stmt.m_params.m_params[i].m_name;

				if (i < func_decl_stmt.m_params.m_params.size() - 1)
					gen.m_output_file << ", ";
			}

			gen.m_output_file << ") {\n";

			for (const auto& i : func_decl_stmt.m_body->m_stmts) {
				gen.gen_stmt(i);
			}

			// not parsing/generating return as stmt, as i only want it available in functions
			//
			if (!func_decl_stmt.is_void) {
				gen.m_output_file << "	";
				gen.m_output_file << "return ";
				gen.gen_expr(*func_decl_stmt.m_return.m_return_expr);
				gen.m_output_file << ";\n";
			}

			gen.m_output_file << "}\n";

		}
	};

	StmtVisitor stmt_visitor{*this};
	std::visit(stmt_visitor, stmt.m_stmt);
}

void Generator::gen_type(const Data_Type& type)
{
	switch (type) {
		case (Data_Type::DOUBLE):
			m_output_file << "double ";
			break;
		case (Data_Type::STRING):
			m_output_file << "std::string ";
			break;
		case (Data_Type::NONE):
			m_output_file << "void ";
			break;
		default: gen_error_l("couldn't gen type");
	}
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
		void operator()(const FuncCallExpr& func_call_expr)
		{
			gen.m_output_file << func_call_expr.m_name;
			gen.m_output_file << "(";

			// turn this into function
			for (std::size_t i{0}; i < func_call_expr.m_args.m_exprs.size(); i++) {
				gen.gen_expr(func_call_expr.m_args.m_exprs[i]);
				if (i < func_call_expr.m_args.m_exprs.size() - 1)
					gen.m_output_file << ", ";
			}

			gen.m_output_file << ")";
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
		if (type1 != Data_Type::NONE && type2 != Data_Type::NONE) {
			gen_error_l("type mismatch");
		}
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

[[nodiscard]] bool Generator::is_var_defined(const VarStmt& var_stmt)
{
	for (auto it{m_existing_vars.rbegin()}; it != m_existing_vars.rend(); it++) {
		if (it->m_name == var_stmt.m_name) {
			return true;
		}
	}

	return false;
}

[[nodiscard]] std::size_t Generator::get_var_index(const VarStmt& var_stmt)
{
	// make this cleaner
	std::size_t i{};

	for (auto it{m_existing_vars.rbegin()}; it != m_existing_vars.rend(); it++, i++) {
		if (it->m_name == var_stmt.m_name) {
			return m_existing_vars.size() - i - 1;
		}
	}

	parse_error_l("cannot get an index of a variable that doesn't exist");
}

[[nodiscard]] Data_Type Generator::existing_vars_lookup(std::string_view name)
{
	for (auto it{m_existing_vars.rbegin()}; it != m_existing_vars.rend(); it++) {
		if (it->m_name == name) {
			return it->m_expr->m_type;
		}
	}

	parse_error_l("couldn't match name with existing variables");
}


[[nodiscard]] bool Generator::is_func_defined(const FuncCallExpr& func_call_expr)
{
	for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++) {
		if (it->m_name == func_call_expr.m_name) {
			return true;
		}
	}

	return false;
}

[[nodiscard]] std::size_t Generator::get_func_index(const FuncCallExpr& func_call_expr)
{
	for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++) {
		if (it->m_name == func_call_expr.m_name) {
			return it - m_existing_funcs.begin();
		}
	}

	gen_error_l("cannot get an index of a function that doesn't exist");
}

[[nodiscard]] Param Generator::existing_funcs_lookup(std::string_view name)
{
	for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++) {
		for (std::size_t i{0}; i < it->m_params.m_params.size(); i++) {
			if (it->m_params.m_params[i].m_name == name) {
				return it->m_params.m_params[i];
			}
		}
	}

	gen_error_l("couldn't match name with existing variables");
}
