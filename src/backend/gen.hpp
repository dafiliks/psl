/* backend/gen.hpp by David Filiks */
/* The generator header for the PsL compiler */

#ifndef GEN_H
#define GEN_H

#include <fstream>
#include <sstream>
#include <array>
#include <cctype>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <variant>

#include "../frontend/parser.hpp"
#include "../compiler/compilation_stage.hpp"
#include "../utils/error_types.hpp"
#include "../utils/vec_ptrs_unwrap.hpp"

/* The main generator class, responsible for converting the valid AST into a C++20 source file */
/* Due to the nature of AQA pseudocode, semantic analysis is performed here */
/* Inherits from CompilationStage as code generation is a compilation stage */
class Generator : public CompilationStage
{

/* Public members */
public:

	/* Functions */

	/* Constructs a Generator object */
	/* Param: const Parser& - a parser object */
	Generator(const Parser& parser);

	/* Executes the gen() function */
	void execute() override;

	/* Generates and stores C++ output code in an appropriate file by traversing the AST  */
	void gen();

	/* Getter function for the abstract syntax tree */
	/* Returns: const AST& - the AST */
	[[nodiscard]] const AST& get_ast() const;

	/* Getter function for the existing variables */
	/* Returns: const std::vector<VarStmt>& - a list of the existing variables */
	[[nodiscard]] const std::vector<VarStmt>& get_existing_vars() const;

	/* Getter function for the existing functions */
	/* Returns: const std::vector<FuncDefStmt>& - a list of the existing functions */
	[[nodiscard]] const std::vector<FuncDefStmt>& get_existing_funcs() const;

	/* Getter function for the existing records */
	/* Returns: const std::vector<RecordStmt>& - a list of the existing records */
	[[nodiscard]] const std::vector<RecordStmt>& get_existing_records() const;

	/* Getter function for the random int count */
	/* Returns: std::size_t - the random int count */
	[[nodiscard]] std::size_t get_random_int_count() const;

	/* Getter function for the output file */
	/* Returns: const std::ofstream& - the output file */
	[[nodiscard]] const std::ofstream& get_output_file() const;

	/* Getter function for the source */
	/* Returns: const std::string& - the source */
	[[nodiscard]] const std::string& get_source() const;

	/* Getter function for the source path */
	/* Returns: const std::string& - the source path */
	[[nodiscard]] const std::string& get_source_path() const;

	/* Getter function for the library stream */
	/* Returns: const std::ostringstream& - the library stream */
	[[nodiscard]] const std::ostringstream& get_lib_stream() const;

	/* Getter function for the record stream */
	/* Returns: const std::ostringstream& - the record stream */
	[[nodiscard]] const std::ostringstream& get_record_stream() const;

	/* Getter function for the function stream */
	/* Returns: const std::ostringstream& - the function stream */
	[[nodiscard]] const std::ostringstream& get_func_stream() const;

	/* Getter function for the main stream */
	/* Returns: const std::ostringstream& - the main stream */
	[[nodiscard]] const std::ostringstream& get_main_stream() const;

	/* Getter function for the current stream */
	/* Returns: const std::ostringstream* - the current stream pointer */
	[[nodiscard]] const std::ostringstream* get_current_stream() const;

	/* Getter function for the used libraries */
	/* Returns: const std::vector<std::string>& - the used library list */
	[[nodiscard]] const std::vector<std::string>& get_existing_libs() const;

/* Private members */
private:

	/* Functions */

	/* Generates output code for any statement */
	/* Param: const Stmt& - the statement */
	void gen_stmt(const Stmt& stmt);

	template <typename T>
	/* Generates output code for any type of arguments */
	/* Param: const std::vector<T>& - the arguments */
	/* Param: const std::string_view - the delimeter */
	void gen_args(const std::vector<T>& args, const std::string_view delim)
	{
		/* Loop through all of the passed arguments */
		for (std::size_t i{}; i < args.size(); i++)
		{
			/* Generate expression output code for each argument */
			gen_expr(*args[i].m_expr);

			/* If the current index is not the last index in the list */
			if (i < args.size() - 1)
			{
				/* Separate the generated arguments with a delimeter */
				*m_current_stream << delim;
			}
		}
	}

	template <typename T>
	/* Generates output code for an arbitrary list of statements */
	/* Param: const std::vector<T>& - the list of statements */
	void gen_stmts(const std::vector<T>& stmts)
	{
		/* Iterate through all of the statements in the list */
		for (const auto& stmt : stmts)
		{
			/* Generate the current statement */
			gen_stmt(Stmt{stmt});
		}
	}

	/* Generates output code for function parameters */
	/* Param: const Params& - the function parameters */
	void gen_params(const Params& params);

	/* Generates output code for any expression */
	/* Param: const Expr& - the expression */
	void gen_expr(const Expr& expr);

	/* Generates output code for an atom expression */
	/* Param: const AtomExpr& - the atom expression */
	void gen_atom_expr(const AtomExpr& atom_expr);

	/* Generates output code for an operator */
	/* Param: const Operator - the operator */
	void gen_op(const Operator op);

	/* Generates output code for any type */
	/* Param: const DataType - the data type */
	void gen_type(const DataType type);

	/* Gets a field type from an attempted record field access */
	/* Param: const FieldAccessStmt& - the field access */
	/* Returns: DataType - the type of the field */
	[[nodiscard]] DataType get_field_type_from_access(const FieldAccessStmt& field_access_stmt) const;

	/* Performs a type check between two types */
	/* Param: const DataType - the first type */
	/* Param: const DataType - the second type */
	void type_check(const DataType type1, const DataType type2) const;

	/* Performs a type check between the arguments of a known function and the passed arguments */
	/* Param: const std::string_view - the name of the function */
	/* Param: const Args& - the arguments to type check against */
	void type_check_func_args(const std::string_view name, const Args& args) const;

	/* Checks that the arguments given to an object creation expression match the record definition */
	/* Param: const std::string_view - the name of the record */
	/* Param: const Args& - the arguments to type check against */
	void type_check_record_args(const std::string_view name, const Args& args) const;

	/* Checks that all of the list elements are of the same type */
	/* Param: const ListExpr& - the list expression */
	void type_check_list(const ListExpr& list_expr) const;

	/* Checks that a specific operator can be used between two types */
	/* Param: const DataType - the first type */
	/* Param: const Operator - the operator */
	/* Param: const DataType - the second type */
	void check_op_valid(const DataType type1, const Operator op, const DataType type2) const;

	/* Returns whether an operator is valid when used between strings */
	/* Param: const Operator - the operator */
	/* Returns: bool - whether the operator is valid when used between strings */
	bool is_op_valid_between_strings(const Operator op) const;

	/* Checks that constant reassignment is not being performed */
	/* Param: const VarStmt& - the variable statement */
	void check_not_constant_reassignment(const VarStmt& var_stmt) const;

	/* Checks that the variable is being reassigned to the same type */
	/* Param: const VarStmt& - the variable statement */
	void check_reassignment_same_type(const VarStmt& var_stmt) const;

	/* Checks that the variable name is in all capitals */
	/* Param: const VarStmt& - the variable statement */
	void check_var_name_capital(const VarStmt& var_stmt) const;

	template <typename T>
	/* Returns whether a list contains an element with a given name */
	/* Param: const std::vector<T>& - the list to search within */
	/* Param: const std::string_view - the name of the element */
	/* Returns: bool - whether the list contains an element with the given name */
	[[nodiscard]] bool does_list_contain_name(const std::vector<T>& list, const std::string_view name) const
	{
		/* Loop through the whole list */
		for (const auto& element : list)
		{
			/* If the current element name matches the desired element name */
			if (element.m_name == name)
			{
				/* Return true as a match was made */
				return true;
			}
		}

		/* Return false as no matches were made */
		return false;
	}

	/* Checks that a variable with a given name exists */
	/* Param: const std::string_view - the variable name */
	void check_var_exists(const std::string_view name) const;

	/* Checks that a function with a given name exists */
	/* Param: const std::string_view - the function name */
	void check_func_exists(const std::string_view name) const;

	/* Checks that a record with a given name exists */
	/* Param: const std::string_view - the record name */
	void check_record_exists(const std::string_view name) const;

	/* Checks that a record has a field with a given name */
	/* Param: const std::string_view - the record name */
	/* Param: const std::string_view - the field name */
	void check_record_has_field(const std::string_view record_name, const std::string_view field_name) const;

	template <typename T>
	/* Counts the number of occurences of elements with a given name within a list */
	/* Param: const std::string_view - the name of the elements to count */
	/* Param: const std::vector<T>& - the list */
	/* Returns: std::size_t - the occurence count */
	[[nodiscard]] std::size_t count_name_occurences_in_list(const std::string_view name, const std::vector<T>& list) const
	{
		/* Store the count (number) of name occurences within the list */
		std::size_t count{};

		/* Loop through the whole list */
		for (const auto& element : list)
		{
			/* If the current element name matches the desired element name */
			if (element.m_name == name)
			{
				/* Increment the occurence count */
				count++;
			}
		}

		/* Return the occurence count */
		return count;
	}

	/* Checks that the function has only been defined once */
	/* Param: const std::string_view - the function name */
	void check_func_defined_once(const std::string_view name) const;

	/* Checks that a record has only been defined once */
	/* Param: const std::string_view - the record name */
	void check_record_defined_once(const std::string_view name) const;

	/* Checks that a function is not void */
	/* Param: const std::string_view - the function name */
	void check_func_non_void(const std::string_view name) const;

	/* Checks that a function has the same argument length as the given argument list */
	/* Param: const std::string_view - the function name */
	/* Param: const Args& - the arguments to compare length to */
	void check_func_arg_length_matches(const std::string_view name, const Args& args) const;

	/* Checks that a record has the same argument length as the given argument list */
	/* Param: const std::string_view - the record name */
	/* Param: const Args& the arguments to compare length to */
	void check_record_arg_length_matches(const std::string_view name, const Args& args) const;

	/* Checks that a given variable is not a list */
	/* Param: const VarStmt& - the variable statement */
	void check_var_not_list(const VarStmt& var_stmt) const;

	/* Checks that an expression is of a certain type */
	/* Param: const Expr& - the expression */
	/* Param: const DataType - the desired data type */
	void check_expr_is_type(const Expr& expr, const DataType data_type) const;

	/* Checks that an expression is not of a certain type */
	/* Param: const Expr& - the expression */
	/* Param: const DataType - the data type not wanted */
	void check_expr_is_not_type(const Expr& expr, const DataType data_type) const;

	/* Looks up an existing variable */
	/* Param: const std::string_view - the name of the variable */
	/* Returns: const VarStmt& - the variable statement */
	[[nodiscard]] const VarStmt& existing_var_lookup(const std::string_view name) const;

	/* Looks up an existing function */
	/* Param: const std::string_view - the name of the function */
	/* Returns: const FuncDefStmt& - the function definition statement */
	[[nodiscard]] const FuncDefStmt& existing_func_lookup(const std::string_view name) const;

	/* Looks up an existing record */
	/* Param: const std::string_view - the name of the record */
	/* Returns: const RecordStmt& - the record statement */
	[[nodiscard]] const RecordStmt& existing_record_lookup(const std::string_view name) const;

	/* Returns whether a specific library is currently loaded or not */
	/* Param: const std::string_view - the library name */
	/* Returns: bool - whether the library is loaded */
	[[nodiscard]] bool is_lib_loaded(const std::string_view name) const;

	/* Loads a given library into the output source code, if not present already */
	/* Param: const std::string - the library name */
	void require_lib(const std::string name);

	/* Changes the output stream */
	/* Param: std::ostringstream& - the new output stream */
	void change_stream(std::ostringstream& new_stream);

	/* Variables */

	AST m_ast{}; /* Holds the AST representation */

	std::vector<VarStmt> m_existing_vars{}; /* The list of existing variables */
	std::vector<FuncDefStmt> m_existing_funcs{}; /* The list of existing functions */
	std::vector<RecordStmt> m_existing_records{}; /* The list of existing records */

	std::size_t m_random_int_count{}; /* A count of how many times the RANDOM_INT() function has been called */

	std::ofstream m_output_file{}; /* A stream to the output source file */
	std::string m_source{}; /* The source contents */
	std::string m_source_path{}; /* The source file path */

	std::ostringstream m_lib_stream{}; /* The library output stream */
	std::ostringstream m_record_stream{}; /* The record output stream */
	std::ostringstream m_func_stream{}; /* The function output stream */
	std::ostringstream m_main_stream{}; /* The main output stream */

	/* A raw pointer is fine here as no dynamic memory allocation is taking place */
	std::ostringstream* m_current_stream{}; /* A pointer to the current output stream */

	std::vector<std::string> m_existing_libs{}; /* A list of the existing libraries */
};

#endif
