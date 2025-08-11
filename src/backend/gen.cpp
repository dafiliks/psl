#include <fstream>
#include <cctype>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <variant>

#include "gen.hpp"
#include "../frontend/ast.hpp"
#include "../utils/error.hpp"
#include "../frontend/parser.hpp"

Generator::Generator(Parser &parser) : m_ast(parser.get_ast()), m_source(parser.get_source()), m_source_path(parser.get_source_path()),
                                       m_existing_vars(parser.get_existing_vars()), m_existing_funcs(parser.get_existing_funcs()),
                                       m_existing_records(parser.get_existing_records()) {}

void Generator::execute()
{
	/* Executes the gen() function */
	gen();
}

void Generator::gen()
{
	const std::string cpp_output_path
	{
		std::filesystem::path{m_source_path}.replace_extension(std::filesystem::path{".cpp"})
	};

	change_stream(m_main_stream);

	*m_current_stream << "int main()\n";
	*m_current_stream << "{\n";

	gen_body(m_ast.m_body);

	*m_current_stream << "	return 0;\n";
	*m_current_stream << "}\n";

	m_output_file.open(cpp_output_path);

	m_output_file << m_lib_stream.str();

	if (!m_lib_stream.str().empty())
	{
		m_output_file << "\n";
	}

	m_output_file << m_record_stream.str();
	m_output_file << m_func_stream.str();
	m_output_file << m_main_stream.str();

	m_output_file.close();
}

void Generator::gen_stmt(const Stmt &stmt)
{
	struct StmtVisitor
	{
		Generator &gen;

		void operator()(const VarStmt &var_stmt)
		{
			if (var_stmt.m_is_reassignment)
			{
				*gen.m_current_stream << var_stmt.m_name << " = ";
				gen.check_not_constant_reassignment(var_stmt);
				gen.check_reassignment_same_type(var_stmt);
			}
			else
			{
				if (var_stmt.m_is_constant)
				{
					*gen.m_current_stream << "const ";
					gen.check_capital_name(var_stmt);
				}

				if (var_stmt.m_is_1d_list)
				{
					gen.require_lib("vector");
					*gen.m_current_stream << "std::vector<";

					if (var_stmt.m_is_2d_list)
					{
						*gen.m_current_stream << "std::vector<";
						gen.gen_type(var_stmt.m_expr->m_type);
						*gen.m_current_stream << ">";
					}
					else
					{
						gen.gen_type(var_stmt.m_expr->m_type);
					}

					*gen.m_current_stream << ">";
					*gen.m_current_stream << " ";
				} else {
					gen.gen_type(var_stmt.m_expr->m_type);
					*gen.m_current_stream << " ";
				}

				*gen.m_current_stream << var_stmt.m_name;
				*gen.m_current_stream << " = ";
			}

			gen.gen_expr(*var_stmt.m_expr);

			*gen.m_current_stream << ";";
		}

		void operator()(const OutputStmt &output_stmt)
		{
			gen.require_lib("iostream");
			*gen.m_current_stream << "std::cout << ";
			gen.gen_output_stmt_args(output_stmt.m_args);
			*gen.m_current_stream << ";";
		}

		void operator()(const FuncDeclStmt &func_decl_stmt)
		{
			gen.change_stream(gen.m_func_stream);

			FuncDeclStmt &table_func_decl_stmt{gen.existing_func_lookup(func_decl_stmt.m_name)};

			gen.check_func_defined_once(table_func_decl_stmt.m_name);

			if (table_func_decl_stmt.m_is_void)
			{
				*gen.m_current_stream << "void ";
			}
			else
			{
				gen.gen_type(table_func_decl_stmt.m_return.m_return_expr->m_type);
				*gen.m_current_stream << " ";
			}

			*gen.m_current_stream << func_decl_stmt.m_name;
			*gen.m_current_stream << "(";

			gen.gen_params(table_func_decl_stmt.m_params);

			*gen.m_current_stream << ")\n";
			*gen.m_current_stream << "{\n";

			gen.gen_body(*table_func_decl_stmt.m_body);

			if (!table_func_decl_stmt.m_is_void)
			{
				*gen.m_current_stream << "	";
				*gen.m_current_stream << "return ";

				gen.gen_expr(*table_func_decl_stmt.m_return.m_return_expr);

				*gen.m_current_stream << ";\n";
			}

			*gen.m_current_stream << "}\n\n";
			gen.change_stream(gen.m_main_stream);
		}

		void operator()(const FuncCallStmt &func_call_stmt)
		{
			*gen.m_current_stream << func_call_stmt.m_name;
			*gen.m_current_stream << "(";

			gen.check_func_defined(func_call_stmt.m_name);
			gen.check_arg_length_matches(func_call_stmt.m_name, func_call_stmt.m_args);
			gen.type_check_func_args(func_call_stmt.m_name, func_call_stmt.m_args);

			gen.gen_args(func_call_stmt.m_args.m_exprs);

			*gen.m_current_stream << ");";
		}

		void operator()(const RepeatUntilStmt &repeat_until_stmt)
		{
			*gen.m_current_stream << "do\n";

			*gen.m_current_stream << "	";
			*gen.m_current_stream << "{\n";

			gen.gen_body(*repeat_until_stmt.m_body);

			*gen.m_current_stream << "	";
			*gen.m_current_stream << "} while (!(";

			gen.gen_expr(*repeat_until_stmt.m_condition_expr);

			*gen.m_current_stream << "));";
		}

		void operator()(const WhileStmt &while_stmt)
		{
			*gen.m_current_stream << "while (";

			gen.gen_expr(*while_stmt.m_condition_expr);
			*gen.m_current_stream << ")\n";

			*gen.m_current_stream << "	";
			*gen.m_current_stream << "{\n";

			gen.gen_body(*while_stmt.m_body);

			*gen.m_current_stream << "	";
			*gen.m_current_stream << "}";
		}

		void operator()(const IfStmt &if_stmt)
		{
			*gen.m_current_stream << "if (";
			gen.gen_expr(*if_stmt.m_condition_expr);
			*gen.m_current_stream << ")\n";

			*gen.m_current_stream << "	";
			*gen.m_current_stream << "{\n";

			gen.gen_body(*if_stmt.m_body);
			*gen.m_current_stream << "	";
			*gen.m_current_stream << "}";
		}

		void operator()(const ElseIfStmt &else_if_stmt)
		{
			*gen.m_current_stream << "else if (";
			gen.gen_expr(*else_if_stmt.m_condition_expr);
			*gen.m_current_stream << ")\n";

			*gen.m_current_stream << "	";
			*gen.m_current_stream << "{\n";

			gen.gen_body(*else_if_stmt.m_body);
			*gen.m_current_stream << "	";
			*gen.m_current_stream << "}";
		}

		void operator()(const ElseStmt &else_stmt)
		{
			*gen.m_current_stream << "else {\n";
			gen.gen_body(*else_stmt.m_body);
			*gen.m_current_stream << "	";
			*gen.m_current_stream << "}";
		}

		void operator()(const ForToStmt &for_to_stmt)
		{
			*gen.m_current_stream << "for (";

			gen.check_var_not_list(for_to_stmt.m_var_stmt);
			gen.check_expr_is_type(*for_to_stmt.m_var_stmt.m_expr, DataType::INT);
			gen.gen_stmt(Stmt{for_to_stmt.m_var_stmt});

			*gen.m_current_stream << " ";

			*gen.m_current_stream << for_to_stmt.m_var_stmt.m_name << " < ";

			gen.check_expr_is_type(*for_to_stmt.m_boundary, DataType::INT);
			gen.gen_expr(*for_to_stmt.m_boundary);

			*gen.m_current_stream << " + 1";

			*gen.m_current_stream << "; ";
			*gen.m_current_stream << for_to_stmt.m_var_stmt.m_name << " += ";

			if (for_to_stmt.m_step)
			{
				gen.check_expr_is_type(*for_to_stmt.m_step, DataType::INT);
				gen.gen_expr(*for_to_stmt.m_step);
			}
			else
			{
				*gen.m_current_stream << "1";
			}

			*gen.m_current_stream << ")\n";

			*gen.m_current_stream << "	";
			*gen.m_current_stream << "{\n";

			gen.gen_body(*for_to_stmt.m_body);

			*gen.m_current_stream << "	";
			*gen.m_current_stream << "}";
		}

		void operator()(const ForInStmt& for_in_stmt)
		{
			*gen.m_current_stream << "for (const auto& ";

			*gen.m_current_stream << for_in_stmt.m_declaration;

			*gen.m_current_stream << " : ";

			gen.check_expr_is_not_type(*for_in_stmt.m_range, DataType::INT);
			gen.check_expr_is_not_type(*for_in_stmt.m_range, DataType::REAL);
			gen.gen_expr(*for_in_stmt.m_range);

			*gen.m_current_stream << ")\n";

			*gen.m_current_stream << "	";
			*gen.m_current_stream << "{\n";

			gen.gen_body(*for_in_stmt.m_body);

			*gen.m_current_stream << "	";
			*gen.m_current_stream << "}\n";
		}
		

		void operator()(const ListAccessStmt& list_access_stmt)
		{
			*gen.m_current_stream << list_access_stmt.m_name;
			*gen.m_current_stream << "[";

			gen.check_expr_is_type(*list_access_stmt.m_row, DataType::INT);
			gen.gen_expr(*list_access_stmt.m_row);

			*gen.m_current_stream << "]";

			if (list_access_stmt.m_col)
			{
				*gen.m_current_stream << "[";

				gen.check_expr_is_type(*list_access_stmt.m_col, DataType::INT);
				gen.gen_expr(*list_access_stmt.m_col);

				*gen.m_current_stream << "]";
			}

			*gen.m_current_stream << " = ";

			gen.check_expr_is_type(*list_access_stmt.m_expr,
			                       gen.existing_var_lookup(list_access_stmt.m_name).m_expr->m_type);

			gen.gen_expr(*list_access_stmt.m_expr);

			*gen.m_current_stream << ";";
		}

		void operator()(const FieldStmt& field_stmt)
		{
			gen.gen_type(field_stmt.m_type);
			*gen.m_current_stream << " ";
			*gen.m_current_stream << field_stmt.m_name;
			*gen.m_current_stream << ";";
		}

		void operator()(const RecordStmt& record_stmt)
		{
			gen.change_stream(gen.m_record_stream);

			gen.check_record_defined_once(record_stmt.m_name);

			*gen.m_current_stream << "struct ";
			*gen.m_current_stream << record_stmt.m_name;
			*gen.m_current_stream << "\n";

			*gen.m_current_stream << "{\n";

			gen.gen_fields(record_stmt);

			*gen.m_current_stream << "};\n\n";

			gen.change_stream(gen.m_main_stream);
		}

		void operator()(const FieldAccessStmt& field_access_stmt)
		{
			gen.check_var_exists(field_access_stmt.m_name);

			*gen.m_current_stream << field_access_stmt.m_name;
			*gen.m_current_stream << ".";

			*gen.m_current_stream << field_access_stmt.m_field_name;

			*gen.m_current_stream << " = ";

			gen.check_expr_is_type(*field_access_stmt.m_expr,
			                       gen.get_field_type_from_access(field_access_stmt));

			gen.gen_expr(*field_access_stmt.m_expr);

			*gen.m_current_stream << ";";
		}
	};

	StmtVisitor stmt_visitor{*this};
	std::visit(stmt_visitor, stmt.m_stmt);
}

void Generator::gen_type(const DataType &type)
{
	switch (type)
	{
	case (DataType::REAL):
		*m_current_stream << "double";
		break;
	case (DataType::INT):
		*m_current_stream << "int";
		break;
	case (DataType::STRING):
		require_lib("string");
		*m_current_stream << "std::string";
		break;
	case (DataType::USER_DEFINED_TYPE):
	case (DataType::UNRESOLVED):
	case (DataType::NONE):
		*m_current_stream << "auto";
		break;
	default:
		gen_error_l("couldn't gen type");
	}
}

void Generator::gen_expr(const Expr &expr)
{
	struct ExprVisitor
	{
		Generator &gen;

		void operator()(const AtomExpr &atom_expr)
		{
			gen.gen_atom_expr(atom_expr);
		}

		void operator()(const BinOpExpr &bin_op_expr)
		{
			const DataType &type1{bin_op_expr.m_lhs->m_type};
			const DataType &type2{bin_op_expr.m_rhs->m_type};

			if (bin_op_expr.m_op != Operator::AND && bin_op_expr.m_op != Operator::OR)
			{
				gen.type_check(type1, type2);
			}

			gen.op_check(type1, bin_op_expr.m_op, type2);

			gen.gen_expr(*bin_op_expr.m_lhs);
			gen.gen_op(bin_op_expr.m_op);
			gen.gen_expr(*bin_op_expr.m_rhs);
		}

		void operator()(const UnaryOpExpr &unary_op_expr)
		{
			gen.gen_op(unary_op_expr.m_op);

			gen.check_expr_is_not_str(*unary_op_expr.m_unary_expr);
			gen.gen_expr(*unary_op_expr.m_unary_expr);
		}

		void operator()(const ParenExpr &paren_expr)
		{
			*gen.m_current_stream << "(";
			gen.gen_expr(*paren_expr.m_expr);
			*gen.m_current_stream << ")";
		}

		void operator()(const ListExpr& list_expr)
		{
			*gen.m_current_stream << "{";

			gen.type_check_list(list_expr);
			gen.gen_args(list_expr.m_exprs);

			*gen.m_current_stream << "}";
		}
	};

	ExprVisitor expr_visitor{*this};
	std::visit(expr_visitor, expr.m_expr);
}

void Generator::gen_atom_expr(const AtomExpr &atom_expr)
{
	struct AtomExprVisitor
	{
		Generator &gen;

		void operator()(const StrExpr &str_expr)
		{
			gen.require_lib("string");
			*gen.m_current_stream << "std::string(\"" << str_expr.m_value << "\")";
		}

		void operator()(const IntExpr &int_expr)
		{
			*gen.m_current_stream << int_expr.m_value;
		}

		void operator()(const RealExpr &float_expr)
		{
			*gen.m_current_stream << float_expr.m_value;
		}

		void operator()(const VarExpr &var_expr)
		{
			*gen.m_current_stream << var_expr.m_name;
		}

		void operator()(const UserInputExpr &user_input_expr)
		{
			gen.require_lib("string");
			gen.require_lib("iostream");
			*gen.m_current_stream << "[](){ std::string s; std::getline(std::cin, s); return s; }()";
		}

		void operator()(const FuncCallExpr &func_call_expr)
		{

			*gen.m_current_stream << func_call_expr.m_name;
			*gen.m_current_stream << "(";

			gen.check_func_defined(func_call_expr.m_name);
			gen.check_func_non_void(func_call_expr.m_name);
			gen.check_arg_length_matches(func_call_expr.m_name, func_call_expr.m_args);
			gen.type_check_func_args(func_call_expr.m_name, func_call_expr.m_args);

			gen.gen_args(func_call_expr.m_args.m_exprs);

			*gen.m_current_stream << ")";
		}

		void operator()(const LenCallExpr &len_call_expr)
		{
			*gen.m_current_stream << "(";

			gen.gen_expr(*len_call_expr.m_expr);

			*gen.m_current_stream << ")";
			*gen.m_current_stream << ".size()";
		}

		void operator()(const PositionCallExpr &position_call_expr)
		{
			*gen.m_current_stream << "(";

			gen.check_expr_is_type(*position_call_expr.m_str_expr, DataType::STRING);
			gen.gen_expr(*position_call_expr.m_str_expr);

			*gen.m_current_stream << ")";
			*gen.m_current_stream << ".find(";

			gen.check_expr_is_type(*position_call_expr.m_char_expr, DataType::STRING);
			gen.gen_expr(*position_call_expr.m_char_expr);

			*gen.m_current_stream << ")";
		}

		void operator()(const SubStrCallExpr &sub_str_call_expr)
		{
			*gen.m_current_stream << "(";

			gen.check_expr_is_type(*sub_str_call_expr.m_str_expr, DataType::STRING);
			gen.gen_expr(*sub_str_call_expr.m_str_expr);

			*gen.m_current_stream << ")";
			*gen.m_current_stream << ".substr(";

			gen.check_expr_is_type(*sub_str_call_expr.m_num1_expr, DataType::INT);
			gen.gen_expr(*sub_str_call_expr.m_num1_expr);

			*gen.m_current_stream << ", ";

			gen.check_expr_is_type(*sub_str_call_expr.m_num2_expr, DataType::INT);
			gen.gen_expr(*sub_str_call_expr.m_num2_expr);

			*gen.m_current_stream << " - 1";
			*gen.m_current_stream << ")";
		}

		void operator()(const StrToIntCallExpr &str_to_int_call_expr)
		{
			*gen.m_current_stream << "std::stoi(";

			gen.check_expr_is_type(*str_to_int_call_expr.m_str_expr, DataType::STRING);
			gen.gen_expr(*str_to_int_call_expr.m_str_expr);

			*gen.m_current_stream << ")";
		}

		void operator()(const StrToRealCallExpr &str_to_real_call_expr)
		{
			*gen.m_current_stream << "std::stod(";

			gen.check_expr_is_type(*str_to_real_call_expr.m_str_expr, DataType::STRING);
			gen.gen_expr(*str_to_real_call_expr.m_str_expr);

			*gen.m_current_stream << ")";
		}

		void operator()(const IntToStrCallExpr &int_to_str_call_expr)
		{
			*gen.m_current_stream << "std::to_string(";

			gen.check_expr_is_type(*int_to_str_call_expr.m_int_expr, DataType::INT);
			gen.gen_expr(*int_to_str_call_expr.m_int_expr);

			*gen.m_current_stream << ")";
		}

		void operator()(const RealToStrCallExpr &real_to_str_call_expr)
		{
			*gen.m_current_stream << "std::to_string(";

			gen.check_expr_is_type(*real_to_str_call_expr.m_real_expr, DataType::REAL);
			gen.gen_expr(*real_to_str_call_expr.m_real_expr);

			*gen.m_current_stream << ")";
		}

		void operator()(const CharToCodeCallExpr &char_to_code_call_expr)
		{
			*gen.m_current_stream << "static_cast<int>((";

			gen.check_expr_is_type(*char_to_code_call_expr.m_char_expr, DataType::CHAR);
			gen.gen_expr(*char_to_code_call_expr.m_char_expr);

			*gen.m_current_stream << ").at(0)";
			*gen.m_current_stream << ")";
		}

		void operator()(const CodeToCharCallExpr &code_to_char_call_expr)
		{
			*gen.m_current_stream << "static_cast<char>(";

			gen.check_expr_is_type(*code_to_char_call_expr.m_int_expr, DataType::INT);
			gen.gen_expr(*code_to_char_call_expr.m_int_expr);

			*gen.m_current_stream << ")";
		}

		void operator()(const RandomIntCallExpr &random_int_call_expr)
		{
			gen.require_lib("random");

			if (gen.m_random_int_count < 1)
			{
				gen.change_stream(gen.m_func_stream);
				*gen.m_current_stream << "int RANDOM_INT(int min, int max) {\n";
				*gen.m_current_stream << "	static std::random_device rd;\n";
				*gen.m_current_stream << "	static std::mt19937 gen(rd());\n";
				*gen.m_current_stream << "	std::uniform_int_distribution<int> distrib(min, max);\n";
				*gen.m_current_stream << "	return distrib(gen);\n";
				*gen.m_current_stream << "}\n";
				gen.change_stream(gen.m_main_stream);

				gen.m_random_int_count++;
			}

			*gen.m_current_stream << "RANDOM_INT(";

			gen.check_expr_is_type(*random_int_call_expr.m_int1_expr, DataType::INT);
			gen.gen_expr(*random_int_call_expr.m_int1_expr);

			*gen.m_current_stream << ", ";

			gen.check_expr_is_type(*random_int_call_expr.m_int2_expr, DataType::INT);
			gen.gen_expr(*random_int_call_expr.m_int2_expr);

			*gen.m_current_stream << ")";
		}

		void operator()(const ListAccessExpr& list_access_expr)
		{
			*gen.m_current_stream << list_access_expr.m_name;

			*gen.m_current_stream << "[";

			gen.check_expr_is_type(*list_access_expr.m_row, DataType::INT);
			gen.gen_expr(*list_access_expr.m_row);

			*gen.m_current_stream << "]";

			if (list_access_expr.m_col)
			{
				*gen.m_current_stream << "[";

				gen.check_expr_is_type(*list_access_expr.m_row, DataType::INT);
				gen.gen_expr(*list_access_expr.m_col);

				*gen.m_current_stream << "]";
			}
		}

		void operator()(const FieldAccessExpr& field_access_expr)
		{
			gen.check_var_exists(field_access_expr.m_name);

			const std::string_view record_name{gen.existing_var_lookup(field_access_expr.m_name).m_record_name};

			gen.check_record_exists(record_name);
			gen.check_record_has_field(record_name, field_access_expr.m_field_name);

			*gen.m_current_stream << field_access_expr.m_name;
			*gen.m_current_stream << ".";
			*gen.m_current_stream << field_access_expr.m_field_name;
		}

		void operator()(const ObjectCreationExpr& object_creation_expr)
		{
			*gen.m_current_stream << object_creation_expr.m_record_name;
			*gen.m_current_stream << "{";

			gen.check_record_defined_once(object_creation_expr.m_record_name);
			gen.check_record_arg_length_matches(object_creation_expr.m_record_name, object_creation_expr.m_args);
			gen.type_check_record_args(object_creation_expr);
			gen.gen_args(object_creation_expr.m_args.m_exprs);

			*gen.m_current_stream << "}";
		}
	};

	AtomExprVisitor atom_expr_visitor{*this};
	std::visit(atom_expr_visitor, atom_expr.m_atom);
}

void Generator::gen_op(const Operator &op)
{
	switch (op)
	{
	case (Operator::PLUS):
		*m_current_stream << "+";
		break;
	case (Operator::MINUS):
		*m_current_stream << "-";
		break;
	case (Operator::MULTIPLY):
		*m_current_stream << "*";
		break;
	case (Operator::DIVIDE):
		*m_current_stream << "/";
		break;
	case (Operator::DIV):
		*m_current_stream << "//";
		break;
	case (Operator::MOD):
		*m_current_stream << "%";
		break;
	case (Operator::LESS_THAN):
		*m_current_stream << "<";
		break;
	case (Operator::GREATER_THAN):
		*m_current_stream << ">";
		break;
	case (Operator::EQUALS):
		*m_current_stream << "==";
		break;
	case (Operator::NOT_EQUALS):
		*m_current_stream << "!=";
		break;
	case (Operator::LESS_THAN_OET):
		*m_current_stream << "<=";
		break;
	case (Operator::GREATER_THAN_OET):
		*m_current_stream << ">=";
		break;
	case (Operator::OR):
		*m_current_stream << "||";
		break;
	case (Operator::AND):
		*m_current_stream << "&&";
		break;
	case (Operator::NOT):
		*m_current_stream << "!";
		break;
	}
}

void Generator::gen_params(const Params &params)
{
	for (std::size_t i{0}; i < params.m_params.size(); i++)
	{
		gen_type(params.m_params[i].m_type);

		*m_current_stream << " ";
		*m_current_stream << params.m_params[i].m_name;

		if (i < params.m_params.size() - 1)
		{
			*m_current_stream << ", ";
		}
	}
}

void Generator::gen_output_stmt_args(const Args &args)
{
	for (std::size_t i{0}; i < args.m_exprs.size(); i++)
	{
		check_expr_is_not_type(args.m_exprs[i], DataType::USER_DEFINED_TYPE);
		gen_expr(args.m_exprs[i]);

		if (i < args.m_exprs.size() - 1)
		{
			*m_current_stream << " << ";
		}
	}
}

void Generator::gen_args(const std::vector<Expr>& v_expr)
{
	for (std::size_t i{0}; i < v_expr.size(); i++)
	{
		gen_expr(v_expr[i]);
		if (i < v_expr.size() - 1)
		{
			*m_current_stream << ", ";
		}
	}
}

void Generator::gen_body(const Body &body)
{
	for (const auto &i : body.m_stmts)
	{
		*m_current_stream << "	";
		gen_stmt(i);
		*m_current_stream << "\n";
	}
}

void Generator::gen_fields(const RecordStmt& record_stmt)
{
	for (const auto& i : record_stmt.m_fields)
	{
		*m_current_stream << "	";
		gen_stmt(Stmt{i});
		*m_current_stream << "\n";
	}
}

DataType Generator::get_field_type_from_access(const FieldAccessStmt& field_access_stmt)
{
	const std::string_view record_name{existing_var_lookup(field_access_stmt.m_name).m_record_name};

	for (const auto& i : m_existing_records)
	{
		if (i.m_name == record_name)
		{
			for (const auto& j : i.m_fields)
			{
				if (j.m_name == field_access_stmt.m_field_name)
				{
					return j.m_type;
				}
			}
		}
	}
}

void Generator::type_check(const DataType &type1, const DataType &type2)
{
	if (type1 != type2 || type1 == DataType::USER_DEFINED_TYPE && type2 == DataType::USER_DEFINED_TYPE)
	{
		gen_error_l("type mismatch");
	}
}

void Generator::type_check_func_args(std::string_view name, const Args &args)
{
	FuncDeclStmt &func_call_stmt{existing_func_lookup(name)};

	for (std::size_t i{0}; i < func_call_stmt.m_params.m_params.size(); i++)
	{
		if (args.m_exprs[i].m_type != func_call_stmt.m_params.m_params[i].m_type)
		{
			gen_error_l("type mismatch when parsing function call arguments");
		}
	}
}

void Generator::type_check_record_args(const ObjectCreationExpr& object_creation_expr)
{
	for (const auto& i : m_existing_records)
	{
		if (i.m_name == object_creation_expr.m_record_name)
		{
			for (std::size_t j{}; j < i.m_fields.size(); j++)
			{
				type_check(i.m_fields[j].m_type,
				           object_creation_expr.m_args.m_exprs[j].m_type);
			}
		}
	}
}

void Generator::type_check_list(const ListExpr& list_expr) const
{
	DataType list_type{list_expr.m_exprs[0].m_type};

	for (const auto& i : list_expr.m_exprs)
	{
		if (i.m_type != list_type)
		{
			gen_error_l("mismatch in types of list");
		}
	}
}

void Generator::op_check(const DataType &type1, const Operator &op, const DataType &type2)
{
	if (type1 == DataType::STRING && type2 == DataType::STRING && op != Operator::PLUS &&
		op != Operator::EQUALS && op != Operator::NOT_EQUALS)
	{
		gen_error_l("only concatenation can be performed between strings");
	}

	if (type1 == DataType::REAL && type2 == DataType::REAL && op == Operator::MOD)
	{
		gen_error_l("mod can only be performed between ints, not reals");
	}
}

void Generator::require_lib(const std::string library)
{
	if (!is_lib_loaded(library))
	{
		m_used_libs.push_back(library);
		m_lib_stream << "#include <" << library << ">\n";
	}
}

void Generator::change_stream(std::ostringstream &new_stream)
{
	m_current_stream = &new_stream;
}

void Generator::check_not_constant_reassignment(const VarStmt &var_stmt)
{
	if (var_stmt.m_is_constant && var_stmt.m_is_reassignment)
	{
		gen_error_l("cannot reassign to constant");
	}
}

void Generator::check_reassignment_same_type(const VarStmt &var_stmt)
{
	if (var_stmt.m_expr->m_type != var_stmt.m_previous_expr->m_type)
	{
		gen_error_l("cannot reassign to a different type");
	}
}

void Generator::check_capital_name(const VarStmt &var_stmt)
{
	if (!std::all_of(var_stmt.m_name.begin(), var_stmt.m_name.end(), [](char c)
					 { return isupper(c); }) &&
		var_stmt.m_is_constant)
	{
		gen_error_l("constants must be named with captial letters");
	}
}

void Generator::check_var_exists(const std::string_view name)
{
	bool found{false};

	for (const auto& i : m_existing_vars)
	{
		if (i.m_name == name)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		gen_error_l("record lacks field");
	}
}

void Generator::check_record_exists(const std::string_view record_name)
{
	bool does_exist{false};

	for (const auto& i : m_existing_records)
	{
		if (i.m_name == record_name)
		{
			does_exist = true;
			break;
		}
	}

	if (!does_exist)
	{
		gen_error_l("record doesn't exist");
	}
}

void Generator::check_record_has_field(const std::string_view record_name, const std::string_view field_name)
{
	bool found{false};

	for (const auto& i : existing_record_lookup(record_name).m_fields)
	{
		if (i.m_name == field_name)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		gen_error_l("record lacks field");
	}
}

void Generator::check_func_defined(const std::string_view name)
{
	bool found{false};

	for (const auto &i : m_existing_funcs)
	{
		if (i.m_name == name)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		gen_error_l("function not defined");
	}
}

void Generator::check_func_defined_once(const std::string_view name)
{
	std::size_t count{};

	for (const auto &i : m_existing_funcs)
	{
		if (i.m_name == name)
			count++;
	}

	for (const auto &i : m_reserved_func_names)
	{
		if (i == name)
			count++;
	}

	if (count != 1)
	{
		gen_error_l("function can only be defined once");
	}
}

void Generator::check_record_defined_once(const std::string_view name)
{
	std::size_t count{};

	for (const auto &i : m_existing_records)
	{
		if (i.m_name == name)
			count++;
	}

	if (count != 1)
	{
		gen_error_l("record can only be defined one time");
	}
}

void Generator::check_func_non_void(const std::string_view name)
{
	FuncDeclStmt &func_decl_stmt{existing_func_lookup(name)};

	if (func_decl_stmt.m_is_void)
	{
		gen_error_l("function call used in expression cannot return nothing");
	}
}

void Generator::check_arg_length_matches(const std::string_view name, const Args &args)
{
	FuncDeclStmt &func_decl_stmt{existing_func_lookup(name)};

	if (args.m_exprs.size() != func_decl_stmt.m_params.m_params.size())
	{
		gen_error_l("amount of arguments given in function call doesn't match definition");
	}
}

void Generator::check_record_arg_length_matches(const std::string_view name, const Args &args)
{
	RecordStmt& record_stmt{existing_record_lookup(name)};

	if (args.m_exprs.size() != record_stmt.m_fields.size())
	{
		gen_error_l("amount of arguments given in function call doesn't match definition");
	}
}

void Generator::check_var_not_list(const VarStmt& var_stmt) const
{
	if (var_stmt.m_is_1d_list || var_stmt.m_is_2d_list)
	{
		gen_error_l("variable used in loop declaration cannot be a list");
	}
}

void Generator::check_expr_is_type(const Expr &expr, const DataType DataType)
{
	if (expr.m_type != DataType)
	{
		gen_error_l("expr doesn't eval to correct data type");
	}
}

void Generator::check_expr_is_not_type(const Expr& expr, DataType DataType) const
{
	if (expr.m_type == DataType)
	{
		gen_error_l("cannot be that data type");
	}
}

void Generator::check_expr_is_not_str(const Expr &expr)
{
	if (expr.m_type == DataType::STRING)
	{
		gen_error_l("expr cannot be string");
	}
}

[[nodiscard]] VarStmt& Generator::existing_var_lookup(std::string_view name)
{
	for (auto it{m_existing_vars.begin()}; it != m_existing_vars.end(); it++)
	{
		if (it->m_name == name)
		{
			return *it;
		}
	}
}

[[nodiscard]] Param &Generator::existing_param_lookup(std::string_view name)
{
	for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++)
	{
		for (std::size_t i{0}; i < it->m_params.m_params.size(); i++)
		{
			if (it->m_params.m_params[i].m_name == name)
			{
				return it->m_params.m_params[i];
			}
		}
	}
}

[[nodiscard]] FuncDeclStmt &Generator::existing_func_lookup(std::string_view name)
{
	for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++)
	{
		if (it->m_name == name)
		{
			return *it;
		}
	}
}

[[nodiscard]] RecordStmt& Generator::existing_record_lookup(std::string_view name)
{
	for (auto it{m_existing_records.begin()}; it != m_existing_records.end(); it++)
	{
		if (it->m_name == name)
		{
			return *it;
		}
	}
}

[[nodiscard]] bool Generator::is_lib_loaded(std::string_view library)
{
	for (const auto &i : m_used_libs)
	{
		if (i == library)
		{
			return true;
		}
	}

	return false;
}
