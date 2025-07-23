#include <fstream>
#include <cctype>
#include <limits>
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
	change_stream(m_main_stream);

	*m_current_stream << "int main()\n";
	*m_current_stream << "{\n";

	gen_body(m_ast.m_body);

	*m_current_stream << "	return 0;\n";
	*m_current_stream << "}\n";

	m_output_file.open(cpp_output_path);

	// actual output to file
	m_output_file << m_lib_stream.str();

	if (!m_lib_stream.str().empty()) {
		m_output_file << "\n";
	}

	m_output_file << m_func_stream.str();
	m_output_file << m_main_stream.str();

	m_output_file.close();
}

void Generator::gen_stmt(const Stmt& stmt)
{
	struct StmtVisitor {
		Generator& gen;

		void operator()(const VarStmt& var_stmt)
		{
			*gen.m_current_stream << "	";

			if (var_stmt.m_is_reassignment) {
				*gen.m_current_stream << var_stmt.m_name << " = ";
				gen.check_not_constant_reassignment(var_stmt);
				gen.check_reassignment_same_type(var_stmt);
			} else {
				if (var_stmt.m_is_constant) {
					*gen.m_current_stream << "const ";
					gen.check_capital_name(var_stmt);
				}

				if (var_stmt.m_expr->m_type == Data_Type::STRING) {
					gen.require_lib("string");
					*gen.m_current_stream << "std::string " << var_stmt.m_name << " = ";
				} else if (var_stmt.m_expr->m_type == Data_Type::DOUBLE) {
					*gen.m_current_stream << "double " << var_stmt.m_name << " = ";
				} else if (var_stmt.m_expr->m_type == Data_Type::NONE) {
					*gen.m_current_stream << "auto " << var_stmt.m_name << " = ";
				}
			}

			gen.gen_expr(*var_stmt.m_expr);

		// add reassignment here after correct gen

			*gen.m_current_stream << ";\n";
		}

		void operator()(const OutputStmt& output_stmt)
		{
			gen.require_lib("iostream");
			*gen.m_current_stream << "	";
			*gen.m_current_stream << "std::cout << ";
			gen.gen_args(output_stmt.m_args, " << ");
			*gen.m_current_stream << ";\n";
		}

		void operator()(const FuncDeclStmt& func_decl_stmt)
		{
			gen.change_stream(gen.m_func_stream);

			FuncDeclStmt& table_func_decl_stmt{gen.existing_func_lookup(func_decl_stmt.m_name)};

			if (table_func_decl_stmt.m_is_void) {
				*gen.m_current_stream << "void ";
			} else {
				// lookup type in table
				gen.gen_type(table_func_decl_stmt.m_return.m_return_expr->m_type);
			}

			*gen.m_current_stream << func_decl_stmt.m_name;
			*gen.m_current_stream << "(";

			gen.gen_params(func_decl_stmt.m_params);

			*gen.m_current_stream << ")\n";
			*gen.m_current_stream << "{\n";

			gen.gen_body(*table_func_decl_stmt.m_body);

			if (!table_func_decl_stmt.m_is_void) {
				*gen.m_current_stream << "	";
				*gen.m_current_stream << "return ";
				gen.gen_expr(*table_func_decl_stmt.m_return.m_return_expr);
				*gen.m_current_stream << ";\n";
			}

			*gen.m_current_stream << "}\n\n";
			gen.change_stream(gen.m_main_stream);
		}

		void operator()(const FuncCallStmt& func_call_stmt)
		{
			*gen.m_current_stream << "	";
			*gen.m_current_stream << func_call_stmt.m_name;
			*gen.m_current_stream << "(";

			gen.check_func_defined(func_call_stmt.m_name);
			gen.check_arg_length_matches(func_call_stmt.m_name, func_call_stmt.m_args);
			gen.type_check_func_args(func_call_stmt.m_name, func_call_stmt.m_args);
			gen.gen_args(func_call_stmt.m_args, ", ");

			*gen.m_current_stream << ");\n";
		}
	};

	StmtVisitor stmt_visitor{*this};
	std::visit(stmt_visitor, stmt.m_stmt);
}

void Generator::gen_type(const Data_Type& type)
{
	switch (type) {
	case (Data_Type::DOUBLE):
		*m_current_stream << "double ";
		break;
	case (Data_Type::STRING):
		*m_current_stream << "std::string ";
		break;
	case (Data_Type::NONE):
		*m_current_stream << "void ";
		break;
	default:
		gen_error_l("couldn't gen type");
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

			gen.op_check(type1, bin_op_expr.m_op, type2);

			gen.gen_expr(*bin_op_expr.m_lhs);
			gen.gen_op(bin_op_expr.m_op);
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
			gen.require_lib("string");

			*gen.m_current_stream << "std::string(" << str_expr.m_value << ")";
		}

		void operator()(const IntExpr& int_expr)
		{
			*gen.m_current_stream << int_expr.m_value;
		}

		void operator()(const FloatExpr& float_expr)
		{
			*gen.m_current_stream << float_expr.m_value;
		}

		void operator()(const VarExpr& var_expr)
		{
			*gen.m_current_stream << var_expr.m_name;
		}

		void operator()(const UserInputExpr& user_input_expr)
		{
			gen.require_lib("string");
			*gen.m_current_stream << "[](){ std::string s; std::getline(std::cin, s); return s; }()";
		}

		void operator()(const FuncCallExpr& func_call_expr)
		{

			*gen.m_current_stream << func_call_expr.m_name;
			*gen.m_current_stream << "(";

			gen.check_func_defined(func_call_expr.m_name);
			gen.check_func_non_void(func_call_expr.m_name);
			gen.check_arg_length_matches(func_call_expr.m_name, func_call_expr.m_args);
			gen.type_check_func_args(func_call_expr.m_name, func_call_expr.m_args);
			gen.gen_args(func_call_expr.m_args, ", ");

			*gen.m_current_stream << ")";
		}
	};

	AtomExprVisitor atom_expr_visitor{*this};
	std::visit(atom_expr_visitor, atom_expr.m_atom);
}

void Generator::gen_op(const Operator& op)
{
	switch (op) {
	case (Operator::PLUS):
		*m_current_stream << " + ";
		break;
	case (Operator::MINUS):
		*m_current_stream << " - ";
		break;
	case (Operator::MULTIPLY):
		*m_current_stream << " * ";
		break;
	case (Operator::DIVIDE):
		*m_current_stream << " / ";
		break;
	case (Operator::DIV):
		*m_current_stream << " DIV ";
		break;
	case (Operator::MOD):
		*m_current_stream << " MOD ";
	}
}

void Generator::gen_params(const Params& params)
{
	for (std::size_t i{0}; i < params.m_params.size(); i++) {
		gen_type(existing_param_lookup(params.m_params[i].m_name).m_type);
		*m_current_stream << params.m_params[i].m_name;

		if (i < params.m_params.size() - 1) {
			*m_current_stream << ", ";
		}
	}
}

void Generator::gen_args(const Args& args, const std::string_view separator)
{
	for (std::size_t i{0}; i < args.m_exprs.size(); i++) {
		gen_expr(args.m_exprs[i]);

		if (i < args.m_exprs.size() - 1) {
			*m_current_stream << separator;
		}
	}
}

void Generator::gen_body(const Body& body)
{
	for (const auto& i : body.m_stmts) {
		gen_stmt(i);
	}
}

void Generator::type_check(const Data_Type& type1, const Data_Type& type2)
{
	if (type1 != type2) {
		gen_error_l("type mismatch");
	}
}

void Generator::type_check_func_args(std::string_view name, const Args& args)
{
	FuncDeclStmt& func_call_stmt{existing_func_lookup(name)};

	for (std::size_t i{0}; i < func_call_stmt.m_params.m_params.size(); i++) {
		if (args.m_exprs[i].m_type != func_call_stmt.m_params.m_params[i].m_type) {
			gen_error_l("type mismatch when parsing function call arguments");
		}
	}
}


void Generator::op_check(const Data_Type& type1, const Operator& op, const Data_Type& type2)
{
	if (type1 == Data_Type::STRING && type2 == Data_Type::STRING && op != Operator::PLUS) {
		gen_error_l("only concatenation can be performed between strings");
	}
}

void Generator::require_lib(const std::string library)
{
	if (!is_lib_loaded(library)) {
		m_used_libs.push_back(library);
		m_lib_stream << "#include <" << library << ">\n";
	}
}

void Generator::change_stream(std::ostringstream& new_stream)
{
	m_current_stream = &new_stream;
}

void Generator::check_not_constant_reassignment(const VarStmt& var_stmt)
{
	if (var_stmt.m_is_constant && var_stmt.m_is_reassignment) {
		gen_error_l("cannot reassign to constant");
	}
}

void Generator::check_reassignment_same_type(const VarStmt& var_stmt)
{
	if (var_stmt.m_expr->m_type != var_stmt.m_previous_expr->m_type) {
		gen_error_l("cannot reassign to a different type");
	}
}

void Generator::check_capital_name(const VarStmt& var_stmt)
{
	if (!std::all_of(var_stmt.m_name.begin(), var_stmt.m_name.end(), [](char c) { return isupper(c); }) && var_stmt.m_is_constant) {
		gen_error_l("constants must be named with captial letters");
	}
}

void Generator::check_func_defined(const std::string_view name)
{
	bool found{false};

	for (const auto& i : m_existing_funcs) {
		if (i.m_name == name) {
			found = true;
		}
	}

	if (!found) {
		gen_error_l("function not defined");
	}
}

void Generator::check_func_non_void(const std::string_view name)
{
	FuncDeclStmt& func_decl_stmt{existing_func_lookup(name)};

	if (func_decl_stmt.m_is_void) {
		gen_error_l("function call used in expression cannot return nothing");
	}
}

void Generator::check_arg_length_matches(const std::string_view name, const Args& args)
{
	FuncDeclStmt& func_decl_stmt{existing_func_lookup(name)};

	if (args.m_exprs.size() != func_decl_stmt.m_params.m_params.size()) {
		gen_error_l("amount of arguments given in function call doesn't match definition");
	}
}

[[nodiscard]] Param& Generator::existing_param_lookup(std::string_view name)
{
	for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++) {
		for (std::size_t i{0}; i < it->m_params.m_params.size(); i++) {
			if (it->m_params.m_params[i].m_name == name) {
				return it->m_params.m_params[i];
			}
		}
	}
}

[[nodiscard]] FuncDeclStmt& Generator::existing_func_lookup(std::string_view name)
{
	for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++) {
		if (it->m_name == name) {
			return *it;
		}
	}
}

[[nodiscard]] bool Generator::is_lib_loaded(std::string_view library)
{
	for (const auto& i : m_used_libs) {
		if (i == library) {
			return true;
		}
	}

	return false;
}
