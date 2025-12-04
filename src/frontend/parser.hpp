/* frontend/parser.hpp by David Filiks */
/* The parser header for the PsL compiler */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <variant>
#include <utility>
#include <cassert>
#include <algorithm>
#include <memory>
#include <format>

#include "lexer.hpp"
#include "ast.hpp"
#include "../compiler/compilation_stage.hpp"
#include "../utils/stack.hpp"
#include "../utils/error_types.hpp"

/* The main parser class, responsible for converting the token stream into an AST */
/* Due to the nature of AQA pseudocode, semantic analysis is performed here */
/* Inherits from CompilationStage as syntactic analysis is a compilation stage */
class Parser : public CompilationStage
{
/* Public members */
public:

    /* Functions */

    /* Constructs a Parser object */
    /* Param: const Lexer& - a lexer object */
    Parser(const Lexer& lexer);

    /* Executes the parse() function */
    void execute() override;

    /* Performs syntactic analysis on the token stream */
    void parse();

    /* Getter function for the AST */
    /* Returns: const AST& - the AST */
    [[nodiscard]] const AST& get_ast() const;

    /* Getter function for the token stream */
    /* Returns: const std::vector<Token>& - the entire token stream */
    [[nodiscard]] const std::vector<Token>& get_tokens() const;

    /* Getter function for the current token index */
    /* Returns: const std::size_t& - the current token index */
    [[nodiscard]] const std::size_t& get_token_index() const;

    /* Getter function for the source contents */
    /* Returns: const std::string& - the source contents */
    [[nodiscard]] const std::string& get_source() const;

    /* Getter function for the source file path */
    /* Returns: const std::string& - the source file path */
    [[nodiscard]] const std::string& get_source_path() const;

    /* Getter function for the existing program variables */
    /* Returns: const std::vector<std::shared_ptr<VarStmt>>& - a list of the existing variables */
    [[nodiscard]] const std::vector<std::shared_ptr<VarStmt>>& get_existing_vars() const;

    /* Getter function for the existing program functions */
    /* Returns: const std::vector<std::shared_ptr<FuncDefStmt>>& - a list of the existing functions */
    [[nodiscard]] const std::vector<std::shared_ptr<FuncDefStmt>>& get_existing_funcs() const;

    /* Getter function for the existing program records */
    /* Returns: const std::vector<std::shared_ptr<RecordStmt>>& - a list of the existing records */
    [[nodiscard]] const std::vector<std::shared_ptr<RecordStmt>>& get_existing_records() const;

    /* Getter function for the unresolved expressions */
    /* Returns: const std::vector<std::pair<std::shared_ptr<Expr>, const std::string>>& - a list of unresolved expression pairs */
    [[nodiscard]] const std::vector<std::pair<std::shared_ptr<Expr>, const std::string>>& get_unresolved_exprs() const;

    /* Getter function for the unresolved declarations */
    /* Returns: const std::vector<std::pair<std::shared_ptr<Expr>, const std::string>>& - a list of unresolved declaration pairs */
    [[nodiscard]] const std::vector<std::pair<std::shared_ptr<Expr>, const std::string>>& get_unresolved_decls() const;

    /* Getter function for the variable scope stack */
    /* Returns: const Scope& - a dynamic stack of variable scope indexes */
    [[nodiscard]] const Stack<std::size_t>& get_var_scope_stack() const;


/* Private members */
private:

    /* Functions */

    /* Parses a statement */
    /* Returns: Stmt - the statement */
    [[nodiscard]] Stmt parse_stmt();

    /* Parses a variable statement */
    /* Returns: VarStmt - the variable statement */
    [[nodiscard]] VarStmt parse_var_stmt();

    /* Parses a field access statement */
    /* Returns: FieldAccessStmt - the field access statement */
    [[nodiscard]] FieldAccessStmt parse_field_access_stmt();

    /* Parses an output statement */
    /* Returns: OutputStmt - the output statement */
    [[nodiscard]] OutputStmt parse_output_stmt();

    /* Parses a function definition statement */
    /* Returns: FuncDefStmt - the function definition statement */
    [[nodiscard]] FuncDefStmt parse_func_def_stmt();

    /* Parses a return statement */
    /* Returns: ReturnStmt - the return statement */
    [[nodiscard]] ReturnStmt parse_return_stmt();

    /* Parses a function call statement */
    /* Returns: FuncCallStmt - the function call statement */
    [[nodiscard]] FuncCallStmt parse_func_call_stmt();

    /* Parses a repeat until statement */
    /* Returns: RepeatUntilStmt - the repeat until statement */
    [[nodiscard]] RepeatUntilStmt parse_repeat_until_stmt();

    /* Parses a while statement */
    /* Returns: WhileStmt - the while statement */
    [[nodiscard]] WhileStmt parse_while_stmt();

    /* Parses an if statement */
    /* Returns: IfStmt - the if statement */
    [[nodiscard]] IfStmt parse_if_stmt();

    /* Parses an else if statement */
    /* Returns: ElseIfStmt - the else if statement */
    [[nodiscard]] ElseIfStmt parse_else_if_stmt();

    /* Parses an else statement */
    /* Returns: ElseStmt - the else statement */
    [[nodiscard]] ElseStmt parse_else_stmt();

    /* Parses a for to statement */
    /* Returns: ForToStmt - the for to statement */
    [[nodiscard]] ForToStmt parse_for_to_stmt();

    /* Parses a for in statement */
    /* Returns: ForInStmt - the for in statement */
    [[nodiscard]] ForInStmt parse_for_in_stmt();

    /* Parses a record statement */
    /* Returns: RecordStmt - the record statement */
    [[nodiscard]] RecordStmt parse_record_stmt();

    /* Parses a field statement */
    /* Returns: FieldStmt - the field statement */
    [[nodiscard]] FieldStmt parse_field_stmt();

    /* Parses a list access statement */
    /* Returns: ListAccessStmt - the list access statement */
    [[nodiscard]] ListAccessStmt parse_list_access_stmt();

    /* Skips over the entire function body, in order to parse it on the second pass */
    void skip_over_function_body();

    /* Parses function definition parameters */
    /* Returns: Params - the function definition parameters */
    [[nodiscard]] Params parse_func_def_params();

    /* Parses function bodies on the second pass, after previous skip */
    void parse_func_bodies_2nd_pass();

    /* Parses all expressions in the unresolved expressions list */
    void parse_unresolved_exprs_2nd_pass();

    /* Checks function call argument length matches function definition */
    /* Param: const std::string_view - the name of the function */
    /* Param: const Args& - the function call arguments */
    void check_arg_count_matches(const std::string_view name, const Args& args) const;

    /* Parses an expression */
    /* Returns: std::shared_ptr<Expr> - the expression */
    [[nodiscard]] std::shared_ptr<Expr> parse_expr();

    /* Parses an atom expression */
    /* Returns: AtomExpr - the atom expression */
    [[nodiscard]] AtomExpr parse_atom();

    /* Parses an integer expression */
    /* Returns: IntExpr - the integer expression */
    [[nodiscard]] IntExpr parse_int_expr();

    /* Parses a real expression */
    /* Returns: RealExpr - the real expression */
    [[nodiscard]] RealExpr parse_real_expr();

    /* Parses a string expression */
    /* Returns: StrExpr - the string expression */
    [[nodiscard]] StrExpr parse_str_expr();

    /* Parses a character expression */
    /* Returns: CharExpr - the character expression */
    [[nodiscard]] CharExpr parse_char_expr();

    /* Parses a variable expression */
    /* Returns: VarExpr - the variable expression */
    [[nodiscard]] VarExpr parse_var_expr();

    /* Parses a unary operator expression */
    /* Returns: UnaryOpExpr - the unary operator expression */
    [[nodiscard]] UnaryOpExpr parse_unary_op_expr();

    /* Parses a field access expression */
    /* Returns: FieldAccessExpr - the field access expression */
    [[nodiscard]] FieldAccessExpr parse_field_access_expr();

    /* Parses a list access expression */
    /* Returns: ListAccessExpr - the list access expression */
    [[nodiscard]] ListAccessExpr parse_list_access_expr();

    /* Parses a function call expression */
    /* Returns: FuncCallExpr - the function call expression */
    [[nodiscard]] FuncCallExpr parse_func_call_expr();

    /* Parses a user input expression */
    /* Returns: UserInputExpr - the user input expression */
    [[nodiscard]] UserInputExpr parse_user_input_expr();

    /* Parses an object creation expression */
    /* Returns: ObjectCreationExpr - the object creation expression */
    [[nodiscard]] ObjectCreationExpr parse_object_creation_expr();

    /* Parses a standard library call expression to LEN() */
    /* Returns: LenCallExpr - the LEN() function call expression */
    [[nodiscard]] LenCallExpr parse_len_call_expr();

    /* Parses a standard library call expression to POSITION() */
    /* Returns: PositionCallExpr - the POSITION() function call expression */
    [[nodiscard]] PositionCallExpr parse_position_call_expr();

    /* Parses a standard library call expression to SUBSTRING() */
    /* Returns: SubStrCallExpr - the SUBSTRING() function call expression */
    [[nodiscard]] SubStrCallExpr parse_sub_str_call_expr();

    /* Parses a standard library call expression to STRING_TO_INT() */
    /* Returns: StrToIntCallExpr - the STRING_TO_INT() function call expression */
    [[nodiscard]] StrToIntCallExpr parse_str_to_int_call_expr();

    /* Parses a standard library call expression to STRING_TO_REAL() */
    /* Returns: StrToRealCallExpr - the STRING_TO_REAL() function call expression */
    [[nodiscard]] StrToRealCallExpr parse_str_to_real_call_expr();

    /* Parses a standard library call expression to INT_TO_STRING() */
    /* Returns: IntToStrCallExpr - the INT_TO_STRING() function call expression */
    [[nodiscard]] IntToStrCallExpr parse_int_to_str_call_expr();

    /* Parses a standard library call expression to REAL_TO_STRING() */
    /* Returns: RealToStrCallExpr - the REAL_TO_STRING() function call expression */
    [[nodiscard]] RealToStrCallExpr parse_real_to_str_call_expr();

    /* Parses a standard library call expression to CHAR_TO_CODE() */
    /* Returns: CharToCodeCallExpr - the CHAR_TO_CODE() function call expression */
    [[nodiscard]] CharToCodeCallExpr parse_char_to_code_call_expr();

    /* Parses a standard library call expression to CODE_TO_CHAR() */
    /* Returns: CodeToCharCallExpr - the CODE_TO_CHAR() function call expression */
    [[nodiscard]] CodeToCharCallExpr parse_code_to_char_call_expr();

    /* Parses a standard library call expression to RANDOM_INT() */
    /* Returns: RandomIntCallExpr - the RANDOM_INT() function call expression */
    [[nodiscard]] RandomIntCallExpr parse_random_int_call_expr();

    /* Parses a body until it encounters one of the stop tokens */
    /* Param: const std::initializer_list<TokenType>& - list of stop tokens */
    /* Returns: Body - the body construct */
    [[nodiscard]] Body parse_body_until(const std::initializer_list<TokenType>& stop_tokens);

    /* Parses field statements until it encounters one of the stop tokens */
    /* Param: const std::initializer_list<TokenType>& - list of stop tokens */
    /* Returns: FieldStmt - the record fields */
    [[nodiscard]] Fields parse_fields_until(const std::initializer_list<TokenType>& stop_tokens);

    template <typename T>
    /* Parses comma seperated expressions */
    /* Returns: std::vector<Element> - the expressions wrapper */
    [[nodiscard]] std::vector<T> parse_cse()
    {
        /* Create a list of T objects */
        std::vector<T> cse{};

        /* Do until the type of the current token is not COMMA */
        do
        {
            /* Consume the current token */
            consume();

            /* Parse and store the expressions separated by commas in wrapper object */
            cse.push_back(T{parse_expr()});

        } while (peek().m_type == TokenType::COMMA); /* Loop condition */

        /* Return the parsed expressions wrapper */
        return cse;
    }

    /* Parses the operator present at the current token index */
    /* Returns: Operator - the operator present */
    [[nodiscard]] Operator parse_op();

    /* Peeks a certain distance away from the current token index */
    /* Param: const std::size_t - distance to peek */
    /* Returns: const Token& - token present at the peek position */
    [[nodiscard]] const Token& peek(const std::size_t distance = 0) const;

    /* Errors out if the token at peek position signifies end of file */
    /* Param: const std::size_t - distance to peek */
    /* Returns: Token - the token present at the peek position */
    const Token& try_peek(const std::size_t distance = 0) const;

    /* Consumes a number of tokens and adjusts the index accordingly */
    /* Param: const std::size_t - distance to consume */
    /* Returns: Token - token present at the new index position */
    const Token& consume(const std::size_t distance = 1);

    /* Errors out if the consumed token does not match the type specified */
    /* Param: const TokenType - the type to check against */
    /* Returns: Token - the token consumed */
    const Token& try_consume(const TokenType type);

    /* Removes variable for existing variables array */
    /* Param: const std::string_view - the name of the variable */
    void remove_var(const std::string_view name);

    /* Checks whether a variable with a certain name is defined previously */
    /* Param: const std::string_view - the name of the variable */
    /* Returns: bool - whether the variable was defined previously */
    [[nodiscard]] bool is_var_defined(const std::string_view name) const;

    /* Looks up an existing variable */
    /* Param: const std::string_view - the name of the variable */
    /* Returns: std::shared_ptr<VarStmt> - the variable statement */
    [[nodiscard]] std::shared_ptr<VarStmt> existing_var_lookup(const std::string_view name) const;

    /* Looks up an existing function */
    /* Param: const std::string_view - the name of the function */
    /* Returns: std::shared_ptr<FuncDefStmt> - the function definition statement */
    std::shared_ptr<FuncDefStmt> existing_func_lookup(const std::string_view name) const;

    /* Checks whether a record with the following name exists */
    /* Param: const std::string_view - the name of suspected record */
    /* Returns: bool - whether any record has the same name */
    [[nodiscard]] bool is_record(const std::string_view name) const;

    /* Checks whether a standard library function with the following name exists */
    /* Param: const std::string_view - the name of suspected standard library function */
    /* Returns: bool - whether any standard library function has the same name */
    [[nodiscard]] bool is_stdlib(const std::string_view name) const;

    /* Checks whether a given token type could be represented as a data type */
    /* Param: const TokenType - the type of the token */
    /* Returns: bool - whether the token type could be represented as a data type */
    [[nodiscard]] bool is_data_type(const TokenType token_type) const;

    /* Deduces function definition parameter types from an argument list */
    /* Param: const std::string_view - the name of the function */
    /* Param: const Args& - the argument list */
    void deduce_func_def_param_types_from_args(const std::string_view name, const Args& args);

    /* Deduces expression data type from a given token */
    /* Param: const Token - the token to deduce from */
    /* Returns: DataType - the data type deduced */
    [[nodiscard]] DataType deduce_expr_type(const Token token) const;

    /* Gets a particular field data type from a field access expression */
    /* Param: const Token - the token containing the variable name */
    /* Param: const Token - the token containing the field name */
    /* Returns: DataType - the field data type */
    [[nodiscard]] DataType deduce_field_type_from_access(const Token name, const Token field) const;

    /* Returns the corresponding data type equivalent for a particular token type */
    /* Param: const TokenType - the token type */
    /* Returns: DataType - the data type equivalent */
    [[nodiscard]] DataType tt_to_dt(const TokenType token_type) const;

    /* Checks whether a token type indicates the presence of a binary operator */
    /* Param: const TokenType - the token type */
    /* Returns: bool - whether the token type indicated the presence of a binary operator */
    [[nodiscard]] bool is_bin_op(const TokenType token_type) const;

    /* Checks whether a token type indicates the presence of a unary operator */
    /* Param: const TokenType - the token type */
    /* Returns: bool - whether the token type indicated the presence of a unary operator */
    [[nodiscard]] bool is_unary(const TokenType token_type) const;

    /* Populates the existing function array with the standard library functions */
    void populate_stdlib_funcs();

    /* Adds a certain standard library function to the existing functions list */
    /* Param: const std::string_view - the name of the function */
    /* const std::initializer_list<DataType>& param_types - the types of the function's parameters in order */
    /* const DataType - the return type for the function */
    void add_stdlib_func(const std::string_view name, const std::initializer_list<DataType>& param_types, const DataType return_type);

    /* Variables */

    AST m_ast{}; /* Holds the AST representation */

    std::vector<Token> m_tokens{}; /* Token stream created by lexer */
    std::size_t m_token_index{}; /* Current index in the token stream */

    std::string m_source{}; /* The source contents */
    std::string m_source_path{}; /* The source file path */

    [[maybe_unused]] std::vector<std::shared_ptr<VarStmt>> m_existing_vars{}; /* A list of existing variables */
    [[maybe_unused]] std::vector<std::shared_ptr<FuncDefStmt>> m_existing_funcs{}; /* A list of existing functions */
    [[maybe_unused]] std::vector<std::shared_ptr<RecordStmt>> m_existing_records{}; /* A list of existing records */

    /* A list of a pair of a unresolved expression and the function name used to resolve it */
    [[maybe_unused]] std::vector<std::pair<std::shared_ptr<Expr>, const std::string>> m_unresolved_exprs{};

    /* A list of a pair of a unresolved declaration and the variable name used to resolve it */
    [[maybe_unused]] std::vector<std::pair<std::shared_ptr<Expr>, const std::string>> m_unresolved_decls{};

    [[maybe_unused]] Stack<std::size_t> m_var_scope_stack{}; /* An index stack, used to manage variable scopes */
};

#endif