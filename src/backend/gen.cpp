/* backend/gen.cpp by David Filiks */
/* The generator implementation for the PsL compiler */

#include "gen.hpp"

Generator::Generator(const Parser& parser)
/* Initialize member variables */
: m_ast(parser.get_ast()),
  m_source(parser.get_source()),
  m_source_path(parser.get_source_path()),
  m_existing_vars(VecPtrsUnwrapper<VarStmt>{parser.get_existing_vars()}.unwrap()),
  m_existing_funcs(VecPtrsUnwrapper<FuncDefStmt>{parser.get_existing_funcs()}.unwrap()),
  m_existing_records(VecPtrsUnwrapper<RecordStmt>{parser.get_existing_records()}.unwrap()) {}

void Generator::execute()
{
    /* Execute the gen() function */
    gen();
}

void Generator::gen()
{
    /* Store the correct C++ output file path, which will be written to */
    const std::string cpp_output_path
    {
        /* Replace the source file path with a ".cpp" extension */
        std::filesystem::path{m_source_path}.replace_extension(std::filesystem::path{".cpp"}).string()
    };

    /* Change the output stream to the main stream */
    change_stream(m_main_stream);

    /* Generate the main function */
    *m_current_stream << "int main()\n";
    *m_current_stream << "{\n";

    /* Generate the program body */
    gen_stmts(m_ast.m_body.m_stmts);

    /* Return zero and close off the main function */
    *m_current_stream << "    return 0;\n";
    *m_current_stream << "}\n";

    /* Open the C++ output file path for writing */
    m_output_file.open(cpp_output_path);

    /* Write the library stream to the output file first */
    m_output_file << m_lib_stream.str();

    /* If the library stream is not empty */
    if (!m_lib_stream.str().empty())
    {
        /* Insert a newline to make it look cleaner */
        m_output_file << "\n";
    }

    /* Write the record stream to the output file */
    m_output_file << m_record_stream.str();

    /* Write the function stream to the output file */
    m_output_file << m_func_stream.str();

    /* Write the main stream to the output file */
    m_output_file << m_main_stream.str();

    /* Close the output file */
    m_output_file.close();
}

[[nodiscard]] const AST& Generator::get_ast() const
{
    /* Return the AST */
    return m_ast;
}

[[nodiscard]] const std::vector<VarStmt>& Generator::get_existing_vars() const
{
    /* Return the existing variables */
    return m_existing_vars;
}

[[nodiscard]] const std::vector<FuncDefStmt>& Generator::get_existing_funcs() const
{
    /* Return the existing functions */
    return m_existing_funcs;
}

[[nodiscard]] const std::vector<RecordStmt>& Generator::get_existing_records() const
{
    /* Return the existing records */
    return m_existing_records;
}

[[nodiscard]] std::size_t Generator::get_random_int_count() const
{
    /* Return the random int count */
    return m_random_int_count;
}

[[nodiscard]] const std::ofstream& Generator::get_output_file() const
{
    /* Return the output file path */
    return m_output_file;
}

[[nodiscard]] const std::string& Generator::get_source() const
{
    /* Return the source contents */
    return m_source;
}

[[nodiscard]] const std::string& Generator::get_source_path() const
{
    /* Return the source file path */
    return m_source_path;
}

[[nodiscard]] const std::ostringstream& Generator::get_lib_stream() const
{
    /* Return the library output stream */
    return m_lib_stream;
}

[[nodiscard]] const std::ostringstream& Generator::get_record_stream() const
{
    /* Return the record output stream */
    return m_record_stream;
}

[[nodiscard]] const std::ostringstream& Generator::get_func_stream() const
{
    /* Return the function output stream */
    return m_func_stream;
}

[[nodiscard]] const std::ostringstream& Generator::get_main_stream() const
{
    /* Return the main output stream */
    return m_main_stream;
}

[[nodiscard]] const std::ostringstream* Generator::get_current_stream() const
{
    /* Return the current stream pointer */
    return m_current_stream;
}

[[nodiscard]] const std::vector<std::string>& Generator::get_existing_libs() const
{
    /* Return the existing library list */
    return m_existing_libs;
}

void Generator::gen_stmt(const Stmt& stmt)
{
    /* Create a statement visitor struct */
    struct StmtVisitor
    {
        void operator()(const VarStmt& var_stmt)
        {
            /* Write a tab for readability */
            *gen.m_current_stream << "    ";

            /* If the variable statement is a reassignment */
            if (var_stmt.m_is_reassignment)
            {
                /* Check that constant reassignment is not being performed */
                gen.check_not_constant_reassignment(var_stmt);

                /* Check that reassignment is being performed to the same type */
                gen.check_reassignment_same_type(var_stmt);

                /* Generate the variable name followed by an equals sign */
                *gen.m_current_stream << var_stmt.m_name << " = ";
            }

            /* If variable reassignment is not being performed */
            else
            {
                /* If the variable is constant */
                if (var_stmt.m_is_constant)
                {
                    /* Check that the variable name is in all capitals */
                    gen.check_var_name_capital(var_stmt);

                    /* Generate the const keyword */
                    *gen.m_current_stream << "const ";
                }

                /* If the variable is a one dimensional list */
                if (var_stmt.m_is_1d_list)
                {
                    /* Require the C++ vector library to be imported */
                    gen.require_lib("vector");

                    /* Generate the start of a vector definition */
                    *gen.m_current_stream << "std::vector<";

                    /* If the variable is a two dimensional list */
                    if (var_stmt.m_is_2d_list)
                    {
                        /* Generate the start of a second vector definition */
                        *gen.m_current_stream << "std::vector<";

                        /* Generate the corresponding C++ data type */
                        gen.gen_type(var_stmt.m_expr->m_type);

                        /* Close off the second vector template initialization */
                        *gen.m_current_stream << ">";
                    }

                    /* If the variable is not a two dimensional list */
                    else
                    {
                        /* Generate the corresponding C++ data type*/
                        gen.gen_type(var_stmt.m_expr->m_type);
                    }

                    /* Close off the vector template initialization */
                    *gen.m_current_stream << "> ";
                }

                /* If the variable is not any list */
                else
                {
                    /* Generate the corresponding C++ data type */
                    gen.gen_type(var_stmt.m_expr->m_type);

                    /* Write a space for readability */
                    *gen.m_current_stream << " ";
                }

                /* Generate the variable name */
                *gen.m_current_stream << var_stmt.m_name;

                /* Write the assignment operator */
                *gen.m_current_stream << " = ";
            }

            /* Generate output code for the variable expression */
            gen.gen_expr(*var_stmt.m_expr);

            /* Write a semicolon to end off the statement */
            *gen.m_current_stream << ";\n";
        }

        void operator()(const OutputStmt& output_stmt)
        {
            /* Require the C++ iostream library to be imported */
            gen.require_lib("iostream");

            /* Write the C++ output stream */
            *gen.m_current_stream << "    std::cout << (";

            /* Generate the output statement arguments, separated by insertion operators */
            gen.gen_args(output_stmt.m_args.m_args, ") << (");

            /* Write a semicolon to end off the statement */
            *gen.m_current_stream << ");\n";
        }

        void operator()(const FuncDefStmt& func_def_stmt)
        {
            /* Change the output stream to the function stream */
            gen.change_stream(gen.m_func_stream);

            /* Store a reference to the function definition statement found in the function table */
            /* This is in case any function properties have changed in the second pass */
            const FuncDefStmt& table_func_def_stmt{gen.existing_func_lookup(func_def_stmt.m_name)};

            /* Check that the function has only been defined once */
            gen.check_func_defined_once(table_func_def_stmt.m_name);

            /* Generate the function return type */
            gen.gen_type(table_func_def_stmt.m_return.m_return_expr->m_type);

            /* Write a single space for readability */
            *gen.m_current_stream << " ";

            /* Write the function name */
            *gen.m_current_stream << table_func_def_stmt.m_name;

            /* Write the start of the function parameter list */
            *gen.m_current_stream << "(";

            /* Generate the function parameter list */
            gen.gen_params(table_func_def_stmt.m_params);

            /* Close off the parameter list and start the function body */
            *gen.m_current_stream << ")\n{\n";

            /* Generate the function body */
            gen.gen_stmts(table_func_def_stmt.m_body->m_stmts);

            /* Close off the function body */
            *gen.m_current_stream << "}\n\n";

            /* Change the output stream back to the main stream */
            gen.change_stream(gen.m_main_stream);
        }

        void operator()(const ReturnStmt& return_stmt)
        {
            /* Write a return statement */
            *gen.m_current_stream << "    return ";

            /* Generate the return statement expression */
            gen.gen_expr(*return_stmt.m_return_expr);

            /* Write a semicolon to end off the statement */
            *gen.m_current_stream << ";\n";
        }

        void operator()(const FuncCallStmt& func_call_stmt)
        {
            /* Write the function name */
            *gen.m_current_stream << "    " << func_call_stmt.m_name;

            /* Write the start of the function parameter list */
            *gen.m_current_stream << "(";

            /* Check that the function has been defined */
            gen.check_func_exists(func_call_stmt.m_name);

            /* Check that the function call argument length matches the function definition */
            gen.check_func_arg_length_matches(func_call_stmt.m_name, func_call_stmt.m_args);

            /* Type check the function call arguments against the function definition parameters */
            gen.type_check_func_args(func_call_stmt.m_name, func_call_stmt.m_args);

            /* Generate the passed function call arguments, separated by commas */
            gen.gen_args(func_call_stmt.m_args.m_args, ", ");

            /* Close off the function call statement*/
            *gen.m_current_stream << ");\n";
        }

        void operator()(const RepeatUntilStmt& repeat_until_stmt)
        {
            /* Write the do keyword */
            *gen.m_current_stream << "    do\n";

            /* Start the do loop body */
            *gen.m_current_stream << "    {\n";

            /* Generate the repeat until statement body */
            gen.gen_stmts(repeat_until_stmt.m_body->m_stmts);

            /* Close off the do loop body and start the condition */
            *gen.m_current_stream << "    } while (!(";

            /* Generate the loop condition */
            gen.gen_expr(*repeat_until_stmt.m_condition_expr);

            /* Close off the condition */
            *gen.m_current_stream << "));\n";
        }

        void operator()(const WhileStmt& while_stmt)
        {
            /* Write the start of the while statement and the loop condition */
            *gen.m_current_stream << "    while (";

            /* Generate the while statement loop condition */
            gen.gen_expr(*while_stmt.m_condition_expr);

            /* Close off the while statement condition */
            *gen.m_current_stream << ")\n";

            /* Start the while statement body */
            *gen.m_current_stream << "    {\n";

            /* Generate the while statement body */
            gen.gen_stmts(while_stmt.m_body->m_stmts);

            /* Close off the while statement body */
            *gen.m_current_stream << "    }\n";
        }

        void operator()(const IfStmt& if_stmt)
        {
            /* Write the if statement and the start of the condition */
            *gen.m_current_stream << "    if (";

            /* Generate the if statement condition */
            gen.gen_expr(*if_stmt.m_condition_expr);

            /* Close off the if statement condition */
            *gen.m_current_stream << ")\n";

            /* Start the if statement body */
            *gen.m_current_stream << "    {\n";

            /* Generate the if statement body */
            gen.gen_stmts(if_stmt.m_body->m_stmts);

            /* Close off the if statement body */
            *gen.m_current_stream << "    }\n";
        }

        void operator()(const ElseIfStmt& else_if_stmt)
        {
            /* Write the else if statement and the start of the condition */
            *gen.m_current_stream << "    else if (";

            /* Generate the else if statement condition */
            gen.gen_expr(*else_if_stmt.m_condition_expr);

            /* Close off the else if condition */
            *gen.m_current_stream << ")\n";

            /* Start the else if statement body */
            *gen.m_current_stream << "    {\n";

            /* Generate the else if statement body */
            gen.gen_stmts(else_if_stmt.m_body->m_stmts);

            /* Close off the else if statement body */
            *gen.m_current_stream << "    }\n";
        }

        void operator()(const ElseStmt& else_stmt)
        {
            /* Write the else statement and the start of the body */
            *gen.m_current_stream << "    else\n    {\n";

            /* Generate the else statement body */
            gen.gen_stmts(else_stmt.m_body->m_stmts);

            /* Close off the else statement body */
            *gen.m_current_stream << "    }\n";
        }

        void operator()(const ForToStmt& for_to_stmt)
        {
            /* Write the start of the for to statement*/
            *gen.m_current_stream << "    for (";

            /* Check that the variable is not a list of any kind */
            gen.check_var_not_list(for_to_stmt.m_var_stmt);

            /* Check that the variable holds an integer expression */
            gen.check_expr_is_type(*for_to_stmt.m_var_stmt.m_expr, DataType::INT);

            /* Generate the variable statement */
            gen.gen_stmt(Stmt{for_to_stmt.m_var_stmt});

            /* Write half of the for to statement condition */
            *gen.m_current_stream << " " << for_to_stmt.m_var_stmt.m_name << "<";

            /* Check that the boundary is an integer expression */
            gen.check_expr_is_type(*for_to_stmt.m_boundary, DataType::INT);

            /* Generate the boundary expression */
            gen.gen_expr(*for_to_stmt.m_boundary);

            /* Write in an additional one iteration to maintain intended functionality as per the specification */
            *gen.m_current_stream << "+1";

            /* Close off the for to condition */
            *gen.m_current_stream << "; ";

            /* Write the start of the loop step */
            *gen.m_current_stream << for_to_stmt.m_var_stmt.m_name << "+=";

            /* If the for to statement step amount has been specified by the programmer */
            if (for_to_stmt.m_step)
            {
                /* Check that the step expression is an integer */
                gen.check_expr_is_type(*for_to_stmt.m_step, DataType::INT);

                /* Generate the step expression*/
                gen.gen_expr(*for_to_stmt.m_step);
            }

            /* If the step amount has not been specified */
            else
            {
                /* Write the default step amount of one */
                *gen.m_current_stream << "1";
            }

            /* Close off the for loop statements */
            *gen.m_current_stream << ")\n";

            /* Start the for to statement body */
            *gen.m_current_stream << "    {\n";

            /* Generate the for to statement body */
            gen.gen_stmts(for_to_stmt.m_body->m_stmts);

            /* Close off the for to statement body */
            *gen.m_current_stream << "    }\n";
        }

        void operator()(const ForInStmt& for_in_stmt)
        {
            /* Write the for in statement and the loop declaration */
            *gen.m_current_stream << "    for (const auto& " << for_in_stmt.m_declaration->m_name;

            /* Write a colon as per the C++ standard */
            *gen.m_current_stream << " : ";

            /* Check that the range expression does not hold a user defined type */
            gen.check_expr_is_not_type(*for_in_stmt.m_range, DataType::USER_DEFINED_TYPE);

            /* Generate the range expression */
            gen.gen_expr(*for_in_stmt.m_range);

            /* Close off the for in statement initialization */
            *gen.m_current_stream << ")\n";

            /* Start the for in statement body */
            *gen.m_current_stream << "    {\n";

            /* Generate the for in statement body */
            gen.gen_stmts(for_in_stmt.m_body->m_stmts);

            /* Close off the for in statement body */
            *gen.m_current_stream << "    }\n";
        }


        void operator()(const ListAccessStmt& list_access_stmt)
        {
            /* Write the list name and the start of a list row index */
            *gen.m_current_stream << "    " << list_access_stmt.m_name << "[";

            /* Check that the list row access expression holds an integer */
            gen.check_expr_is_type(*list_access_stmt.m_row, DataType::INT);

            /* Generate the list row access expression */
            gen.gen_expr(*list_access_stmt.m_row);

            /* Close off the list row access */
            *gen.m_current_stream << "]";

            /* If the list access statement has a valid col (is 2D) */
            if (list_access_stmt.m_col)
            {
                /* Write the start of a list col index */
                *gen.m_current_stream << "[";

                /* Check that the list col access expression holds an integer */
                gen.check_expr_is_type(*list_access_stmt.m_col, DataType::INT);

                /* Generate the list col access expression */
                gen.gen_expr(*list_access_stmt.m_col);

                /* Close off list col access */
                *gen.m_current_stream << "]";
            }

            /* Write the assignment operator */
            *gen.m_current_stream << " = ";

            /* Check that the new expression at the index specified is of the same type as the previous one */
            gen.check_expr_is_type(*list_access_stmt.m_expr, gen.existing_var_lookup(list_access_stmt.m_name).m_expr->m_type);

            /* Generate the list access statement expression */
            gen.gen_expr(*list_access_stmt.m_expr);

            /* Write a semicolon to end off the statement */
            *gen.m_current_stream << ";\n";
        }

        void operator()(const FieldStmt& field_stmt)
        {
            /* Write a tab for readability */
            *gen.m_current_stream << "    ";

            /* Generate the type of the field statement */
            gen.gen_type(field_stmt.m_type);

            /* Write the name of the field and close off the statement */
            *gen.m_current_stream << " " << field_stmt.m_name << ";\n";
        }

        void operator()(const RecordStmt& record_stmt)
        {
            /* Change the output stream to the record stream */
            gen.change_stream(gen.m_record_stream);

            /* Check that the record has only been defined once */
            gen.check_record_defined_once(record_stmt.m_name);

            /* Write the record definition */
            *gen.m_current_stream << "struct " << record_stmt.m_name << "\n";

            /* Start the record statement body */
            *gen.m_current_stream << "{\n";

            /* Generate the record statement fields */
            gen.gen_stmts(record_stmt.m_fields.m_fields);

            /* Close off the record statement body */
            *gen.m_current_stream << "};\n\n";

            /* Change the output stream back to the main stream */
            gen.change_stream(gen.m_main_stream);
        }

        void operator()(const FieldAccessStmt& field_access_stmt)
        {
            /* Check that the variable which the field access is attempted upon exists */
            gen.check_var_exists(field_access_stmt.m_name);

            /* Check that the field accessed exists */
            gen.check_record_has_field(gen.existing_var_lookup(field_access_stmt.m_name).m_record_name, field_access_stmt.m_field_name);

            /* Write the field access statement and the assignment operator */
            *gen.m_current_stream << "    " << field_access_stmt.m_name << "." << field_access_stmt.m_field_name << " = ";

            /* Check that the new field expression is of the same type as the one specified in the record definition */
            gen.check_expr_is_type(*field_access_stmt.m_expr, gen.get_field_type_from_access(field_access_stmt));

            /* Generate the new field expression */
            gen.gen_expr(*field_access_stmt.m_expr);

            /* Write a semicolon to end off the statement */
            *gen.m_current_stream << ";\n";
        }

        Generator& gen; /* A reference to a code generator */
    };

    /* Create a statement visitor */
    StmtVisitor stmt_visitor{*this};

    /* Visit the appropriate statement generation function(s) */
    std::visit(stmt_visitor, stmt.m_stmt);
}

void Generator::gen_params(const Params& params)
{
    /* Loop through all of the parameters passed to the function */
    for (std::size_t i{}; i < params.m_params.size(); i++)
    {
        /* Generate the type of the current parameter */
        gen_type(params.m_params[i].m_type);

        /* Write a space followed by the parameter name */
        *m_current_stream << " " << params.m_params[i].m_name;

        /* If the current parameter index is not the last index */
        if (i < params.m_params.size() - 1)
        {
            /* Write a comma to separate the parameters */
            *m_current_stream << ", ";
        }
    }
}

void Generator::gen_expr(const Expr& expr)
{
    /* Create an expression visitor struct */
    struct ExprVisitor
    {
        void operator()(const AtomExpr& atom_expr)
        {
            /* Generate the atom expression */
            gen.gen_atom_expr(atom_expr);
        }

        void operator()(const BinOpExpr& bin_op_expr)
        {
            /* If the operator used is not a boolean operator that takes two operands */
            if (bin_op_expr.m_op != Operator::AND && bin_op_expr.m_op != Operator::OR)
            {
                /* Check that both the lhs and rhs are of the same type */
                gen.type_check(bin_op_expr.m_lhs->m_type, bin_op_expr.m_rhs->m_type);
            }

            /* Check that the operator is valid, considering the types of the lhs and rhs */
            gen.check_op_valid(bin_op_expr.m_lhs->m_type, bin_op_expr.m_op, bin_op_expr.m_rhs->m_type);

            /* Generate the lhs expression */
            gen.gen_expr(*bin_op_expr.m_lhs);

            /* Generate the operator used */
            gen.gen_op(bin_op_expr.m_op);

            /* Generate the rhs expression */
            gen.gen_expr(*bin_op_expr.m_rhs);
        }

        void operator()(const UnaryOpExpr& unary_op_expr)
        {
            /* Generate the unary expression operator */
            gen.gen_op(unary_op_expr.m_op);

            /* Check that the expression is not of type USER_DEFINED_TYPE */
            gen.check_expr_is_not_type(*unary_op_expr.m_unary_expr, DataType::USER_DEFINED_TYPE);

            /* Generate the unary expression */
            gen.gen_expr(*unary_op_expr.m_unary_expr);
        }

        void operator()(const ParenExpr& paren_expr)
        {
            /* Write the open parenthesis */
            *gen.m_current_stream << "(";

            /* Generate the expression inside the parentheses */
            gen.gen_expr(*paren_expr.m_expr);

            /* Write the closed parenthesis */
            *gen.m_current_stream << ")";
        }

        void operator()(const ListExpr& list_expr)
        {
            /* Start off the initializer list */
            *gen.m_current_stream << "{";

            /* Check that all elements in the list are of the same type */
            gen.type_check_list(list_expr);

            /* Generate the list arguments, separated by commas */
            gen.gen_args(list_expr.m_list.m_list, ", ");

            /* Close off the initializer list */
            *gen.m_current_stream << "}";
        }

        Generator& gen; /* A reference to a code generator */
    };

    /* Create an expression visitor */
    ExprVisitor expr_visitor{*this};

    /* Visit the appropriate expression generation function(s) */
    std::visit(expr_visitor, expr.m_expr);
}

void Generator::gen_atom_expr(const AtomExpr& atom_expr)
{
    /* Create a atom expression visitor struct */
    struct AtomExprVisitor
    {
        void operator()(const StrExpr& str_expr)
        {
            /* Require the C++ string library to be imported */
            gen.require_lib("string");

            /* Write the string expression value as a string object */
            *gen.m_current_stream << "std::string(\"" << str_expr.m_value << "\")";
        }

        void operator()(const CharExpr& char_expr)
        {
            /* Write the char expression value */
            *gen.m_current_stream << "'" << char_expr.m_value << "'";
        }

        void operator()(const IntExpr& int_expr)
        {
            /* Write the integer expression value */
            *gen.m_current_stream << int_expr.m_value;
        }

        void operator()(const RealExpr& real_expr)
        {
            /* Write the real expression value */
            *gen.m_current_stream << real_expr.m_value;
        }

        void operator()(const VarExpr& var_expr)
        {
            /* Write the variable expression name */
            *gen.m_current_stream << var_expr.m_name;
        }

        void operator()(const UserInputExpr& user_input_expr)
        {
            /* Require the C++ string library to be imported */
            gen.require_lib("string");

            /* Require the C++ iostream library to be imported */
            gen.require_lib("iostream");

            /* Write a lambda to retrieve user input via getline and cin */
            *gen.m_current_stream << "[](){ std::string s; std::getline(std::cin, s); return s; }()";
        }

        void operator()(const FuncCallExpr& func_call_expr)
        {
            /* Write the function name and the start of the argument list */
            *gen.m_current_stream << func_call_expr.m_name << "(";

            /* Check that the function has been defined */
            gen.check_func_exists(func_call_expr.m_name);

            /* Check that the function is not void, as void functions cannot be used in expressions */
            gen.check_func_non_void(func_call_expr.m_name);

            /* Check that the function call argument length matches the function definition */
            gen.check_func_arg_length_matches(func_call_expr.m_name, func_call_expr.m_args);

            /* Type check the function call arguments against the function definition parameters */
            gen.type_check_func_args(func_call_expr.m_name, func_call_expr.m_args);

            /* Generate the passed function call arguments, separated by commas */
            gen.gen_args(func_call_expr.m_args.m_args, ", ");

            /* Close off the function call argument list */
            *gen.m_current_stream << ")";
        }

        void operator()(const LenCallExpr& len_call_expr)
        {
            /* Write an open parenthesis */
            *gen.m_current_stream << "(";

            /* Generate the only argument given */
            gen.gen_expr(*len_call_expr.m_expr);

            /* Write a closed parenthesis */
            *gen.m_current_stream << ")";

            /* Get the length (size) of the expression passed */
            *gen.m_current_stream << ".size()";
        }

        void operator()(const PositionCallExpr& position_call_expr)
        {
            /* Write an open parenthesis */
            *gen.m_current_stream << "(";

            /* Check that the first argument passed is of type STRING */
            gen.check_expr_is_type(*position_call_expr.m_str_expr, DataType::STRING);

            /* Generate the given string expression */
            gen.gen_expr(*position_call_expr.m_str_expr);

            /* Write a closed parenthesis */
            *gen.m_current_stream << ")";

            /* Use the find member to locate the position of the occurrence */
            *gen.m_current_stream << ".find(";

            /* Check that the second argument passed is of type CHAR */
            gen.check_expr_is_type(*position_call_expr.m_char_expr, DataType::CHAR);

            /* Generate the given char expression*/
            gen.gen_expr(*position_call_expr.m_char_expr);

            /* Write a closed parenthesis */
            *gen.m_current_stream << ")";
        }

        void operator()(const SubStrCallExpr& sub_str_call_expr)
        {
            /* Write an open parenthesis */
            *gen.m_current_stream << "(";

            /* Check that the first argument passed is of type STRING */
            gen.check_expr_is_type(*sub_str_call_expr.m_str_expr, DataType::STRING);

            /* Generate the given string expression */
            gen.gen_expr(*sub_str_call_expr.m_str_expr);

            /* Write a closed parenthesis */
            *gen.m_current_stream << ")";

            /* Use the substr member function to create a substring */
            *gen.m_current_stream << ".substr(";

            /* Check that the first argument passed is of type INT */
            gen.check_expr_is_type(*sub_str_call_expr.m_num1_expr, DataType::INT);

            /* Generate the given integer expression */
            gen.gen_expr(*sub_str_call_expr.m_num1_expr);

            /* separate the arguments with a comma */
            *gen.m_current_stream << ", ";

            /* Check that the first argument passed is of type INT */
            gen.check_expr_is_type(*sub_str_call_expr.m_num2_expr, DataType::INT);

            /* Generate the given integer expression */
            gen.gen_expr(*sub_str_call_expr.m_num2_expr);

            /* End off substring one character short to maintain intended functionality as per the specification */
            *gen.m_current_stream << "-1";

            /* Write a closed parenthesis */
            *gen.m_current_stream << ")";
        }

        void operator()(const StrToIntCallExpr& str_to_int_call_expr)
        {
            /* Use the stoi function to convert a string to an integer */
            *gen.m_current_stream << "std::stoi(";

            /* Check that the first argument passed is of type STRING */
            gen.check_expr_is_type(*str_to_int_call_expr.m_str_expr, DataType::STRING);

            /* Generate the string expression */
            gen.gen_expr(*str_to_int_call_expr.m_str_expr);

            /* Write a closed parenthesis */
            *gen.m_current_stream << ")";
        }

        void operator()(const StrToRealCallExpr& str_to_real_call_expr)
        {
            /* Use the stod function to convert a string to a real */
            *gen.m_current_stream << "std::stod(";

            /* Check that the first argument passed is of type STRING */
            gen.check_expr_is_type(*str_to_real_call_expr.m_str_expr, DataType::STRING);

            /* Generate the string expression */
            gen.gen_expr(*str_to_real_call_expr.m_str_expr);

            /* Write a closed parenthesis */
            *gen.m_current_stream << ")";
        }

        void operator()(const IntToStrCallExpr& int_to_str_call_expr)
        {
            /* Use the to string function to convert an integer to a string */
            *gen.m_current_stream << "std::to_string(";

            /* Check that the first argument passed is of type INT */
            gen.check_expr_is_type(*int_to_str_call_expr.m_int_expr, DataType::INT);

            /* Generate the integer expression */
            gen.gen_expr(*int_to_str_call_expr.m_int_expr);

            /* Write a closed parenthesis */
            *gen.m_current_stream << ")";
        }

        void operator()(const RealToStrCallExpr& real_to_str_call_expr)
        {
            /* Use the to string function to convert a real to a string */
            *gen.m_current_stream << "std::to_string(";

            /* Check that the first argument passed is of type REAL */
            gen.check_expr_is_type(*real_to_str_call_expr.m_real_expr, DataType::REAL);

            /* Generate the real expression */
            gen.gen_expr(*real_to_str_call_expr.m_real_expr);

            /* Write a closed parenthesis */
            *gen.m_current_stream << ")";
        }

        void operator()(const CharToCodeCallExpr& char_to_code_call_expr)
        {
            /* Write a static cast, to convert the char into an integer */
            *gen.m_current_stream << "static_cast<int>(";

            /* Check that the first argument passed is of type CHAR */
            gen.check_expr_is_type(*char_to_code_call_expr.m_char_expr, DataType::CHAR);

            /* Generate the char expression */
            gen.gen_expr(*char_to_code_call_expr.m_char_expr);

            /* Write a closed parenthesis */
            *gen.m_current_stream << ")";
        }

        void operator()(const CodeToCharCallExpr& code_to_char_call_expr)
        {
            /* Write a static cast, to convert the integer (code) into a char */
            *gen.m_current_stream << "static_cast<char>(";

            /* Check that the first argument passed is of type INT */
            gen.check_expr_is_type(*code_to_char_call_expr.m_int_expr, DataType::INT);

            /* Generate the integer expression */
            gen.gen_expr(*code_to_char_call_expr.m_int_expr);

            /* Write a closed parenthesis */
            *gen.m_current_stream << ")";
        }

        void operator()(const RandomIntCallExpr& random_int_call_expr)
        {
            /* Require the C++ random library to be imported */
            gen.require_lib("random");

            /* If this is the first time that the RANDOM_INT() function has been called */
            if (gen.m_random_int_count < 1)
            {
                /* Change the output stream to the function stream */
                gen.change_stream(gen.m_func_stream);

                /* Write the function return type, name, and parameters */
                *gen.m_current_stream << "int RANDOM_INT(int min, int max)\n";

                /* Start off the function body */
                *gen.m_current_stream << "{\n";

                /* Create a uniformly distributed integer random number generator */
                *gen.m_current_stream << "    static std::random_device rd{};\n";

                /* Create a Mersenne Twister pseudo-random generator, initialized with rd */
                *gen.m_current_stream << "    static std::mt19937 gen{rd{}};\n";

                /* Produce a uniform distribution on the closed interval [min, max] */
                *gen.m_current_stream << "    std::uniform_int_distribution<int> distrib{min, max};\n";

                /* Return the randomly generated number */
                *gen.m_current_stream << "    return distrib{gen};\n";

                /* Close off the function body */
                *gen.m_current_stream << "}\n";

                /* Change the output stream back to the main stream */
                gen.change_stream(gen.m_main_stream);

                /* Increment the function usage count, so that the function implementation is not redefined */
                gen.m_random_int_count++;
            }

            /* Write the RANDOM_INT() function call */
            *gen.m_current_stream << "RANDOM_INT(";

            /* Check that the first argument passed is of type INT */
            gen.check_expr_is_type(*random_int_call_expr.m_int1_expr, DataType::INT);

            /* Generate the integer expression */
            gen.gen_expr(*random_int_call_expr.m_int1_expr);

            /* separate the arguments with a comma */
            *gen.m_current_stream << ", ";

            /* Check that the second argument passed is of type INT */
            gen.check_expr_is_type(*random_int_call_expr.m_int2_expr, DataType::INT);

            /* Generate the integer expression */
            gen.gen_expr(*random_int_call_expr.m_int2_expr);

            /* Write a closed parenthesis */
            *gen.m_current_stream << ")";
        }

        void operator()(const ListAccessExpr& list_access_expr)
        {
            /* Write the list name and the start of a list row index */
            *gen.m_current_stream << list_access_expr.m_name << "[";

            /* Check that the list row access expression holds an integer */
            gen.check_expr_is_type(*list_access_expr.m_row, DataType::INT);

            /* Generate the list row access expression */
            gen.gen_expr(*list_access_expr.m_row);

            /* Close off the list row access */
            *gen.m_current_stream << "]";

            /* If the list access expression has a valid col (is 2D) */
            if (list_access_expr.m_col)
            {
                /* Write the start of a list col index */
                *gen.m_current_stream << "[";

                /* Check that the list col access expression holds an integer */
                gen.check_expr_is_type(*list_access_expr.m_col, DataType::INT);

                /* Generate the list col access expression */
                gen.gen_expr(*list_access_expr.m_col);

                /* Close off list col access */
                *gen.m_current_stream << "]";
            }
        }

        void operator()(const FieldAccessExpr& field_access_expr)
        {
            /* Check that the variable which the field access is attempted upon exists */
            gen.check_var_exists(field_access_expr.m_name);

            /* Check that the field accessed exists */
            gen.check_record_has_field(gen.existing_var_lookup(field_access_expr.m_name).m_record_name, field_access_expr.m_field_name);

            /* Write the field access expression */
            *gen.m_current_stream << field_access_expr.m_name << "." << field_access_expr.m_field_name;
        }

        void operator()(const ObjectCreationExpr& object_creation_expr)
        {
            /* Write the object record name and the start of the list initialization */
            *gen.m_current_stream << object_creation_expr.m_record_name << "{";

            /* Check that the record exists */
            gen.check_record_exists(object_creation_expr.m_record_name);

            /* Check that the number of fields in the record matches the argument list given */
            gen.check_record_arg_length_matches(object_creation_expr.m_record_name, object_creation_expr.m_args);

            /* Check that the types of the record fields match the argument types */
            gen.type_check_record_args(object_creation_expr.m_record_name, object_creation_expr.m_args);

            /* Generate the object creation expression arguments, separated by commas */
            gen.gen_args(object_creation_expr.m_args.m_args, ", ");

            /* Close off the list initialization */
            *gen.m_current_stream << "}";
        }

        Generator& gen; /* A reference to a code generator */
    };

    /* Create an atom expression visitor */
    AtomExprVisitor atom_expr_visitor{*this};

    /* Visit the appropriate atom expression generation function(s) */
    std::visit(atom_expr_visitor, atom_expr.m_atom);
}

void Generator::gen_op(const Operator op)
{
    /* Switch through all of the possible operators */
    switch (op)
    {
        /* If the operator is ADDITION */
        case (Operator::ADDITION):
            /* Write the addition operator */
            *m_current_stream << "+";

            /* Break from the switch case */
            break;

        /* If the operator is SUBTRACTION */
        case (Operator::SUBTRACTION):
            /* Write the subtraction operator */
            *m_current_stream << "-";

            /* Break from the switch case */
            break;

        /* If the operator is MULTIPLICATION */
        case (Operator::MULTIPLICATION):
            /* Write the multiplication operator */
            *m_current_stream << "*";

            /* Break from the switch case */
            break;

        /* If the operator is DIVISION */
        case (Operator::DIVISION):
            /* Fall through to the next case */

        /* If the operator is DIV */
        case (Operator::DIV):
            /* Write the division operator */
            *m_current_stream << "/";

            /* Break from the switch case */
            break;

        /* If the operator is MOD */
        case (Operator::MOD):
            /* Write the modulus operator */
            *m_current_stream << "%";

            /* Break from the switch case */
            break;

        /* If the operator is LESS_THAN */
        case (Operator::LESS_THAN):
            /* Write the less than operator */
            *m_current_stream << "<";

            /* Break from the switch case */
            break;

        /* If the operator is GREATER_THAN */
        case (Operator::GREATER_THAN):
            /* Write the greater than operator */
            *m_current_stream << ">";

            /* Break from the switch case */
            break;

        /* If the operator is EQUALS */
        case (Operator::EQUALS):
            /* Write the equality operator */
            *m_current_stream << "==";

            /* Break from the switch case */
            break;

        /* If the operator is NOT_EQUALS */
        case (Operator::NOT_EQUALS):
            /* Write the not equal to operator */
            *m_current_stream << "!=";

            /* Break from the switch case */
            break;

        /* If the operator is LESS_THAN_OR_EQUAL_TO */
        case (Operator::LESS_THAN_OET):
            /* Write the less than or equal to operator */
            *m_current_stream << "<=";

            /* Break from the switch case */
            break;

        /* If the operator is GREATER_THAN_OR_EQUAL_TO */
        case (Operator::GREATER_THAN_OET):
            /* Write the greater than or equal to operator */
            *m_current_stream << ">=";

            /* Break from the switch case */
            break;

        /* If the operator is OR */
        case (Operator::OR):
            /* Write the logical or operator */
            *m_current_stream << "||";

            /* Break from the switch case */
            break;

        /* If the operator is AND */
        case (Operator::AND):
            /* Write the logical and operator */
            *m_current_stream << "&&";

            /* Break from the switch case */
            break;

        /* If the operator is NOT */
        case (Operator::NOT):
            /* Write the logical not operator */
            *m_current_stream << "!";

            /* Break from the switch case */
            break;

        /* If no matches occur */
        default:
            /* Throw gen error */
            throw GenError
            {
                "an operator could not be generated"
            };
    }
}

void Generator::gen_type(const DataType type)
{
    /* Switch through all of the possible data types */
    switch (type)
    {
        /* If the data type is INT */
        case (DataType::INT):
            /* Write the int keyword */
            *m_current_stream << "int";

            /* Break from the switch case */
            break;

        /* If the data type is INT */
        case (DataType::REAL):
            /* Write the double keyword */
            *m_current_stream << "double";

            /* Break from the switch case */
            break;

        /* If the data type is STRING */
        case (DataType::STRING):
            /* Require the C++ string library to be imported */
            require_lib("string");

            /* Write the std::string object type */
            *m_current_stream << "std::string";

            /* Break from the switch case */
            break;

        /* If the data type is CHAR */
        case (DataType::CHAR):
            /* Write the char keyword */
            *m_current_stream << "char";

            /* Break from the switch case */
            break;

        /* If the data type is UNRESOLVED */
        case (DataType::UNRESOLVED):
            /* Fall through to the next case */

        /* If the data type is USER_DEFINED_TYPE */
        case (DataType::USER_DEFINED_TYPE):
            /* Write the auto keyword */
            *m_current_stream << "auto";

            /* Break from the switch case */
            break;

        /* If the data type is VOID */
        case (DataType::VOID):
            /* Write the void keyword */
            *m_current_stream << "void";

            /* Break from the switch case */
            break;

        /* If no matches occur */
        default:
            /* Throw gen error */
            throw GenError
            {
                "data type could not be generated"
            };
    }
}

DataType Generator::get_field_type_from_access(const FieldAccessStmt& field_access_stmt) const
{
    /* Loop through all of the existing records */
    for (const auto& record : m_existing_records)
    {
        /* If the current record name matches an existing variable record name */
        if (record.m_name == existing_var_lookup(field_access_stmt.m_name).m_record_name)
        {
            /* Loop through the current record's fields */
            for (const auto& field : record.m_fields.m_fields)
            {
                /* If the current field name matches with the field being accessed */
                if (field.m_name == field_access_stmt.m_field_name)
                {
                    /* Return the field type */
                    return field.m_type;
                }
            }
        }
    }

    /* If no matches occur */
    /* Throw gen error */
    throw GenError
    {
        "record \"" +
        existing_var_lookup(field_access_stmt.m_name).m_record_name +
        "\" does not contain field \"" +
        field_access_stmt.m_field_name +
        "\""
    };
}

void Generator::type_check(const DataType type1, const DataType type2) const
{
    /* If there is a mismatch between the two types */
    if (type1 != type2)
    {
        /* If the type mismatch is not between a char and a string, which is permitted */
        if (type1 != DataType::STRING && type2 != DataType::CHAR ||
            type1 != DataType::CHAR   && type2 != DataType::STRING)
        {
            /* Throw gen error */
            throw GenError
            {
                "type mismatch between \"" +
                std::string{dt_to_string(type1)} +
                "\" and \"" +
                std::string{dt_to_string(type2)} +
                "\""
            };
        }
    }
}

void Generator::type_check_func_args(const std::string_view name, const Args& args) const
{
    /* Store a reference to the function parameters */
    const std::vector<Param>& func_params{existing_func_lookup(name).m_params.m_params};

    /* Iterate through the parameters of the given function */
    for (std::size_t i{}; i < func_params.size(); i++)
    {
        /* Type check each function argument type against the corresponding parameter type */
        type_check(args.m_args[i].m_expr->m_type, func_params[i].m_type);
    }
}

void Generator::type_check_record_args(const std::string_view name, const Args& args) const
{
    /* Store a reference to the record fields */
    const std::vector<FieldStmt>& record_fields{existing_record_lookup(name).m_fields.m_fields};

    /* Iterate through the fields of the given record */
    for (std::size_t i{}; i < record_fields.size(); i++)
    {
        /* Type check each record field type against the corresponding argument type */
        type_check(record_fields[i].m_type, args.m_args[i].m_expr->m_type);
    }
}

void Generator::type_check_list(const ListExpr& list_expr) const
{
    /* Store a reference to the list type, which is the type of the first list element */
    DataType& list_type{list_expr.m_list.m_list[0].m_expr->m_type};

    /* Loop through all of the list elements */
    for (const auto& element : list_expr.m_list.m_list)
    {
        /* Type check each element type against the list type */
        type_check(element.m_expr->m_type, list_type);
    }
}

void Generator::check_op_valid(const DataType type1, const Operator op, const DataType type2) const
{
    /* If both types are STRING and the operator used between them is not valid */
    if (type1 == DataType::STRING && type2 == DataType::STRING && !is_op_valid_between_strings(op))
    {
        /* Throw gen error */
        throw GenError
        {
            "the \"" +
            std::string{op_to_string(op)} +
            "\" operator cannot be used between strings"
        };
    }

    /* If either type is not INT and the operator used is MOD */
    else if ((type1 != DataType::INT || type2 != DataType::INT) && op == Operator::MOD)
    {
        /* Throw gen error */
        throw GenError
        {
            "the \"MOD\" operator can only be used between integers"
        };
    }

    /* If either type is not INT and the operator used is DIV */
    else if ((type1 != DataType::INT || type2 != DataType::INT) && op == Operator::DIV)
    {
        /* Throw gen error */
        throw GenError
        {
            "the \"DIV\" operator can only be used between integers"
        };
    }
}

bool Generator::is_op_valid_between_strings(const Operator op) const
{
    /* Return whether the operator is valid when used between strings */
    return op == Operator::ADDITION ||
           op == Operator::LESS_THAN ||
           op == Operator::GREATER_THAN ||
           op == Operator::EQUALS ||
           op == Operator::NOT_EQUALS ||
           op == Operator::LESS_THAN_OET ||
           op == Operator::GREATER_THAN_OET;
}

void Generator::check_not_constant_reassignment(const VarStmt& var_stmt) const
{
    /* If the variable is constant and is being reassigned */
    if (var_stmt.m_is_constant && var_stmt.m_is_reassignment)
    {
        /* Throw gen error */
        throw GenError
        {
            "constant variable \"" +
            var_stmt.m_name +
            "\" cannot be reassigned"
        };
    }
}

void Generator::check_reassignment_same_type(const VarStmt& var_stmt) const
{
    /* If the current variable expression type does not match the previous variable expression type */
    if (var_stmt.m_expr->m_type != var_stmt.m_previous_expr->m_type)
    {
        /* Throw gen error */
        throw GenError
        {
            "variable \"" +
            var_stmt.m_name +
            "\" of type \"" +
            std::string{dt_to_string(var_stmt.m_expr->m_type)} +
            "\" cannot be reassigned to different type \"" +
            std::string{dt_to_string(var_stmt.m_previous_expr->m_type)} +
            "\""
        };
    }
}

void Generator::check_var_name_capital(const VarStmt& var_stmt) const
{
    /* If the variable is constant */
    if (var_stmt.m_is_constant)
    {
        /* If not all of the characters in the variable name are capital */
        if (!std::all_of(var_stmt.m_name.begin(), var_stmt.m_name.end(), [](char c) { return isupper(c) || c == '_'; }))
        {
            /* Throw gen error */
            throw GenError
            {
                "constant variable \"" +
                var_stmt.m_name +
                "\" needs to be named using all capital letters"
            };
        }
    }
}

void Generator::check_var_exists(const std::string_view name) const
{
    /* If the existing variables list does not contain an element with the given name */
    if (!does_list_contain_name(m_existing_vars, name))
    {
        /* Throw gen error */
        throw GenError
        {
            "variable \"" +
            std::string{name} +
            "\" does not exist"
        };
    }
}

void Generator::check_func_exists(const std::string_view name) const
{
    /* If the existing functions list does not contain an element with the given name */
    if (!does_list_contain_name(m_existing_funcs, name))
    {
        /* Throw gen error */
        throw GenError
        {
            "function \"" +
            std::string{name} +
            "\" does not exist"
        };
    }
}

void Generator::check_record_exists(const std::string_view name) const
{
    /* If the existing records list does not contain an element with the given name */
    if (!does_list_contain_name(m_existing_records, name))
    {
        /* Throw gen error */
        throw GenError
        {
            "record \"" +
            std::string{name} +
            "\" does not exist"
        };
    }
}

void Generator::check_record_has_field(const std::string_view record_name, const std::string_view field_name) const
{
    /* If the record fields do not contain a field with the given name */
    if (!does_list_contain_name(existing_record_lookup(record_name).m_fields.m_fields, field_name))
    {
        /* Throw gen error */
        throw GenError
        {
            "record \"" +
            std::string{record_name} +
            "\" lacks field \"" +
            std::string{field_name} +
            "\""
        };
    }
}

void Generator::check_func_defined_once(const std::string_view name) const
{
    /* If the function name occurs more or less than once in the existing functions list */
    if (count_name_occurences_in_list(name, m_existing_funcs) != 1)
    {
        /* Throw gen error */
        throw GenError
        {
            "function \"" +
            std::string{name} +
            "\" needs to be defined once"
        };
    }
}

void Generator::check_record_defined_once(const std::string_view name) const
{
    /* If the record name occurs more or less than once in the existing records list */
    if (count_name_occurences_in_list(name, m_existing_records) != 1)
    {
        /* Throw gen error */
        throw GenError
        {
            "record \"" +
            std::string{name} +
            "\" needs to be defined once"
        };
    }
}

void Generator::check_func_non_void(const std::string_view name) const
{
    /* If the function return type is VOID */
    if (existing_func_lookup(name).m_return.m_return_expr->m_type == DataType::VOID)
    {
        /* Throw gen error */
        throw GenError
        {
            "function \"" +
            std::string{name} +
            "\" returns void - it cannot be used in an expression"
        };
    }
}

void Generator::check_func_arg_length_matches(const std::string_view name, const Args& args) const
{
    /* Store a reference to the function parameters */
    const std::vector<Param>& func_params{existing_func_lookup(name).m_params.m_params};

    /* If the argument count is not equal to the function parameter count */
    if (args.m_args.size() != func_params.size())
    {
        /* Throw gen error */
        throw GenError
        {
            "function \"" +
            std::string{name} +
            "\" expects \"" +
            std::to_string(func_params.size()) +
            "\" arguments, got \"" +
            std::to_string(args.m_args.size()) +
            "\""
        };
    }
}

void Generator::check_record_arg_length_matches(const std::string_view name, const Args& args) const
{
    /* Store a reference to the record fields */
    const std::vector<FieldStmt>& record_fields{existing_record_lookup(name).m_fields.m_fields};

    /* If the argument count is not equal to the record field count */
    if (args.m_args.size() != record_fields.size())
    {
        /* Throw gen error */
        throw GenError
        {
            "record construction \"" +
            std::string{name} +
            "\" expects \"" +
            std::to_string(record_fields.size()) +
            "\" arguments, got \"" +
            std::to_string(args.m_args.size()) +
            "\""
        };
    }
}

void Generator::check_var_not_list(const VarStmt& var_stmt) const
{
    /* If the variable is a list */
    if (var_stmt.m_is_1d_list || var_stmt.m_is_2d_list)
    {
        /* Throw gen error */
        throw GenError
        {
            "variable \"" +
            var_stmt.m_name +
            "\" is used in a loop declaration - it cannot be a list"
        };
    }
}

void Generator::check_expr_is_type(const Expr& expr, const DataType data_type) const
{
    /* If the expression is not of the desired type */
    if (expr.m_type != data_type)
    {
        /* Throw gen error */
        throw GenError
        {
            "expected \"" +
            std::string{dt_to_string(expr.m_type)} +
            "\", got \"" +
            std::string{dt_to_string(data_type)} +
            "\""
        };
    }
}

void Generator::check_expr_is_not_type(const Expr& expr, const DataType data_type) const
{
    /* If the expression is of the given type */
    if (expr.m_type == data_type)
    {
        /* Throw gen error */
        throw GenError
        {
            "type \"" +
            std::string{dt_to_string(data_type)} +
            "\" is prohibited here"
        };
    }
}

[[nodiscard]] const VarStmt& Generator::existing_var_lookup(const std::string_view name) const
{
    /* Iterate through all of the existing variables */
    for (auto it{m_existing_vars.begin()}; it != m_existing_vars.end(); it++)
    {
        /* If the current variable name matches the name given */
        if (it->m_name == name)
        {
            /* Return the current variable statement */
            return *it;
        }
    }

    /* If nothing was found */
    /* Throw gen error */
    throw GenError
    {
        "no existing variable is named \"" +
        std::string{name} +
        "\""
    };
}

[[nodiscard]] const FuncDefStmt& Generator::existing_func_lookup(const std::string_view name) const
{
    /* Iterate through all of the existing functions */
    for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++)
    {
        /* If the current function name matches the name given */
        if (it->m_name == name)
        {
            /* Return the current function definition statement */
            return *it;
        }
    }

    /* If nothing was found */
    /* Throw gen error */
    throw GenError
    {
        "no existing function is named \"" +
        std::string{name} +
        "\""
    };
}

[[nodiscard]] const RecordStmt& Generator::existing_record_lookup(const std::string_view name) const
{
    /* Iterate through all of the existing records */
    for (auto it{m_existing_records.begin()}; it != m_existing_records.end(); it++)
    {
        /* If the current record name matches the name given */
        if (it->m_name == name)
        {
            /* Return the current record statement */
            return *it;
        }
    }

    /* If nothing was found */
    /* Throw gen error */
    throw GenError
    {
        "no existing record is named \"" +
        std::string{name} +
        "\""
    };
}

[[nodiscard]] bool Generator::is_lib_loaded(const std::string_view name) const
{
    /* Loop through all of the existing loaded libraries */
    for (const auto& library : m_existing_libs)
    {
        /* If the current library name matches with the given name */
        if (library == name)
        {
            /* Return true as the library is loaded */
            return true;
        }
    }

    /* Return false as the library is not loaded */
    return false;
}

void Generator::require_lib(const std::string name)
{
    /* If the library has not been imported previously */
    if (!is_lib_loaded(name))
    {
        /* Write the library include preprocessor directive */
        m_lib_stream << "#include <" << name << ">\n";

        /* Append the library to the existing libraries list */
        m_existing_libs.push_back(name);
    }
}

void Generator::change_stream(std::ostringstream& new_stream)
{
    /* Set the current output stream to the new output stream */
    m_current_stream =& new_stream;
}
