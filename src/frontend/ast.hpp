/* frontend/ast.hpp by David Filiks */
/* The ast header for the PsL compiler */

#ifndef AST_HPP
#define AST_HPP

#include <variant>
#include <vector>
#include <string>
#include <memory>

/* Enum class for every data type in the language */
enum class DataType
{
	INT, /* Represents an integer type */
	REAL, /* Represents a real type */
	STRING, /* Represents a string type */
	CHAR, /* Represents a character type */
	UNRESOLVED, /* Represents an unresolved type */
	USER_DEFINED_TYPE, /* Represents a user defined type */
	VOID, /* Represents a void type */
};

/* Returns the corresponding string equivalent for a particular data type */
/* Param: const DataType - the data type */
/* Returns: std::string_view - the string equivalent */
inline std::string_view dt_to_string(const DataType type)
{
	/* Switch through all of the possible data types */
	switch (type)
	{
		/* Return the corresponding string */
		case (DataType::INT):               return "integer";
		case (DataType::REAL):              return "real";
		case (DataType::STRING):            return "string";
		case (DataType::CHAR):              return "char";
		case (DataType::UNRESOLVED):        return "unresolved";
		case (DataType::USER_DEFINED_TYPE): return "user defined type";
		case (DataType::VOID):              return "void";

		/* No match made */
		default:                            return "no matching type found";
	}
}

/* Enum class for every type of operator */
enum class Operator
{
	ADDITION, /* Represents the addition "+" character */
	SUBTRACTION, /* Represents the subtraction "-" character */
	MULTIPLICATION, /* Represents the multiplication "*" character */
	DIVISION, /* Represents the division "/" character */
	DIV, /* Represents the "DIV" operator */
	MOD, /* Represents the "MOD" operator */
	LESS_THAN, /* Represents the less than "<" character */
	GREATER_THAN, /* Represents the greater than ">" character */
	EQUALS, /* Represents the equals "=" character */
	NOT_EQUALS,  /* Represents the not equals "!=" operator */
	LESS_THAN_OET, /* Represents the less than or equal to "<=" operator */
	GREATER_THAN_OET, /* Represents the greater than or equal to ">=" operator */
	AND, /* Represents the "AND" operator */
	OR, /* Represents the "OR" operator */
	NOT, /* Represents the "NOT" operator */
};

/* Returns the corresponding string equivalent for a particular operator type */
/* Param: const Operator - the operator */
/* Returns: std::string_view - the string equivalent */
inline std::string_view op_to_string(const Operator op)
{
	/* Switch through all of the possible operators */
	switch (op)
	{
		/* Return the corresponding string */
		case (Operator::ADDITION):             return "addition";
		case (Operator::SUBTRACTION):          return "subtraction";
		case (Operator::MULTIPLICATION):       return "multiplication";
		case (Operator::DIVISION):             return "division";
		case (Operator::DIV):                  return "DIV";
		case (Operator::MOD):                  return "DIV";
		case (Operator::LESS_THAN):            return "less than";
		case (Operator::GREATER_THAN):         return "greater than";
		case (Operator::EQUALS):               return "equals";
		case (Operator::NOT_EQUALS):           return "not equals";
		case (Operator::LESS_THAN_OET):        return "less than or equal to";
		case (Operator::GREATER_THAN_OET):     return "greater than or equal to";
		case (Operator::AND):                  return "AND";
		case (Operator::OR):                   return "OR";
		case (Operator::NOT):                  return "NOT";

		/* No match made */
		default:                               return "no matching operator found";
	}
}

struct Expr; /* Declaration for the Expr struct */
struct StrExpr; /* Declaration for the StrExpr struct */
struct VarExpr; /* Declaration for the VarExpr struct */
struct Scope; /* Declaration for the Scope struct */
struct Body; /* Declaration for the Body struct */
struct RelationalOpExpr; /* Declaration for the RelationOpExpr struct */
struct AtomExpr; /* Declaration for the AtomExpr struct */

/* Struct representing a variable statement */
struct VarStmt
{
	std::string m_name{}; /* The name of the variable */
	std::shared_ptr<Expr> m_expr{}; /* The expression that the variable holds */

	std::shared_ptr<Expr> m_previous_expr{}; /* The variable's previous expression */

	bool m_is_constant{}; /* Is the variable constant */
	bool m_is_reassignment{}; /* Has the variable been involved in a reassignment operation */

	bool m_is_1d_list{}; /* Is the variable a one dimensional list */
	bool m_is_2d_list{}; /* Is the variable a two dimensional list */

	std::string m_record_name{}; /* The record name if the variable is a user defined object */
};

/* Struct representing a parameter */
struct Param
{
	std::string m_name{}; /* The name of the parameter */
	DataType m_type{}; /* The data type of the parameter */
};

/* Struct representing a collection of parameters */
struct Params
{
	std::vector<Param> m_params{}; /* The list of parameters */
};

/* Struct representing a single argument */
struct Arg
{
	std::shared_ptr<Expr> m_expr{}; /* The argument expression */
};

/* Struct representing a collection of arguments */
struct Args
{
	std::vector<Arg> m_args{}; /* The list of argument expressions */
};

/* Struct representing an output statement */
struct OutputStmt
{
	Args m_args{}; /* The arguments to be output */
};

/* Struct representing a return statement */
struct ReturnStmt
{
	std::shared_ptr<Expr> m_return_expr{}; /* The return expression */
};

/* Struct representing a function definition */
struct FuncDefStmt
{
	std::string m_name{}; /* The name of the function */
	Params m_params{}; /* The list of parameters */

	std::size_t m_token_index_start{}; /* The token index where the function starts */

	std::shared_ptr<Body> m_body{}; /* The body of the function */
	ReturnStmt m_return{}; /* The return statement */

	bool m_is_called{}; /* Has the function been called */
};

/* Struct representing a function call */
struct FuncCallStmt
{
	std::string m_name{}; /* The name of the function */
	Args m_args{}; /* The list of arguments passed to the function */
};

/* Struct representing a repeat until statement */
struct RepeatUntilStmt
{
	std::shared_ptr<Expr> m_condition_expr{}; /* The loop condition */
	std::shared_ptr<Body> m_body{}; /* The body of the loop */
};

/* Struct representing a while loop statement */
struct WhileStmt
{
	std::shared_ptr<Expr> m_condition_expr{}; /* The loop condition */
	std::shared_ptr<Body> m_body{}; /* The body of the loop */
};

/* Struct representing an if statement */
struct IfStmt
{
	std::shared_ptr<Expr> m_condition_expr{}; /* The condition for the if statement */
	std::shared_ptr<Body> m_body{}; /* The body of the if statement */
};

/* Struct representing an else if statement */
struct ElseIfStmt
{
	std::shared_ptr<Expr> m_condition_expr{}; /* The condition for the else if statement */
	std::shared_ptr<Body> m_body{}; /* The body of the else if statement */
};

/* Struct representing an else statement */
struct ElseStmt
{
	std::shared_ptr<Body> m_body{}; /* The body of the else statement */
};

/* Struct representing a for to loop statement */
struct ForToStmt
{
	VarStmt m_var_stmt{}; /* The variable declaration or initialization */
	std::shared_ptr<Expr> m_boundary{}; /* The boundary expression */
	std::shared_ptr<Expr> m_step{}; /* The optional step expression */
	std::shared_ptr<Body> m_body{}; /* The body of the loop */
};

/* Struct representing a for in loop statement */
struct ForInStmt
{
	std::shared_ptr<VarStmt> m_declaration{}; /* The variable declaration */
	std::shared_ptr<Expr> m_range{}; /* The range expression to iterate over */
	std::shared_ptr<Body> m_body{}; /* The body of the loop */
};

/* Struct representing a list access assignment statement */
struct ListAccessStmt
{
	std::string m_name{}; /* The name of the list variable */
	std::shared_ptr<Expr> m_row{}; /* The row index expression */
	std::shared_ptr<Expr> m_col{}; /* The column index expression (if 2D) */
	std::shared_ptr<Expr> m_expr{}; /* The value to assign */
};

/* Struct representing a field declaration inside a record */
struct FieldStmt
{
	std::string m_name{}; /* The name of the field */
	DataType m_type{}; /* The data type of the field */
};

/* Struct representing a collection of fields */
struct Fields
{
	std::vector<FieldStmt> m_fields{}; /* The list of field declarations */
};

/* Struct representing a record declaration */
struct RecordStmt
{
	std::string m_name{}; /* The name of the record */
	Fields m_fields{}; /* The fields that belong to the record */
};

/* Struct representing a field access assignment */
struct FieldAccessStmt
{
	std::string m_name{}; /* The name of the record variable */
	std::string m_field_name{}; /* The name of the field being accessed */
	std::shared_ptr<Expr> m_expr{}; /* The expression assigned to the field */
};

/* Struct representing a statement */
struct Stmt
{
	std::variant<VarStmt,
	             OutputStmt,
	             FuncDefStmt,
		     ReturnStmt,
	             FuncCallStmt,
	             RepeatUntilStmt,
	             WhileStmt,
	             IfStmt,
	             ElseIfStmt,
	             ElseStmt,
	             ForToStmt,
	             ForInStmt,
	             ListAccessStmt,
	             FieldStmt,
	             RecordStmt,
	             FieldAccessStmt> m_stmt{}; /* The statement variant */
};

/* Struct representing a body (block of statements) */
struct Body
{
	std::vector<Stmt> m_stmts{}; /* The list of statements in the body */
};

/* Struct representing an integer expression */
struct IntExpr
{
	int m_value{}; /* The integer value */
};

/* Struct representing a real number expression */
struct RealExpr
{
	double m_value{}; /* The real number value */
};

/* Struct representing a string expression */
struct StrExpr
{
	std::string m_value{}; /* The string value */
};

/* Struct representing a character expression */
struct CharExpr
{
	char m_value{}; /* The character value */
};

/* Struct representing a variable expression */
struct VarExpr
{
	std::string m_name{}; /* The name of the variable */
};

/* Struct representing a user input expression */
struct UserInputExpr
{};

/* Struct representing a function call expression */
struct FuncCallExpr
{
	std::string m_name{}; /* The name of the function */
	Args m_args{}; /* The arguments passed to the function */
};

/* Struct representing a LEN() function call */
struct LenCallExpr
{
	std::shared_ptr<Expr> m_expr{}; /* The expression to get the length of */
};

/* Struct representing a POSITION() function call */
struct PositionCallExpr
{
	std::shared_ptr<Expr> m_str_expr{}; /* The string expression */
	std::shared_ptr<Expr> m_char_expr{}; /* The character expression */
};

/* Struct representing a SUBSTRING() function call */
struct SubStrCallExpr
{
	std::shared_ptr<Expr> m_num1_expr{}; /* The starting position */
	std::shared_ptr<Expr> m_num2_expr{}; /* The length of the substring */
	std::shared_ptr<Expr> m_str_expr{}; /* The string expression */
};

/* Struct representing a STRING_TO_INT() function call */
struct StrToIntCallExpr
{
	std::shared_ptr<Expr> m_str_expr{}; /* The string to convert to integer */
};

/* Struct representing a STRING_TO_REAL() function call */
struct StrToRealCallExpr
{
	std::shared_ptr<Expr> m_str_expr{}; /* The string to convert to real */
};

/* Struct representing an INT_TO_STRING() function call */
struct IntToStrCallExpr
{
	std::shared_ptr<Expr> m_int_expr{}; /* The integer to convert to string */
};

/* Struct representing a REAL_TO_STRING() function call */
struct RealToStrCallExpr
{
	std::shared_ptr<Expr> m_real_expr{}; /* The real number to convert to string */
};

/* Struct representing a CHAR_TO_CODE() function call */
struct CharToCodeCallExpr
{
	std::shared_ptr<Expr> m_char_expr{}; /* The character to convert to ASCII code */
};

/* Struct representing a CODE_TO_CHAR() function call */
struct CodeToCharCallExpr
{
	std::shared_ptr<Expr> m_int_expr{}; /* The ASCII code to convert to character */
};

/* Struct representing a RANDOM_INT() function call */
struct RandomIntCallExpr
{
	std::shared_ptr<Expr> m_int1_expr{}; /* The minimum integer value */
	std::shared_ptr<Expr> m_int2_expr{}; /* The maximum integer value */
};

/* Struct representing a list access expression */
struct ListAccessExpr
{
	std::string m_name{}; /* The name of the list variable */
	std::shared_ptr<Expr> m_row{}; /* The row index */
	std::shared_ptr<Expr> m_col{}; /* The column index (if 2D) */
};

/* Struct representing a field access expression */
struct FieldAccessExpr
{
	std::string m_name{}; /* The name of the record variable */
	std::string m_field_name{}; /* The name of the field */
};

/* Struct representing an object creation */
struct ObjectCreationExpr
{
	std::string m_record_name{}; /* The name of the record type */
	Args m_args{}; /* The arguments used for initialization */
};

/* Struct representing an atom expression */
struct AtomExpr
{
	std::variant<IntExpr,
	             RealExpr,
	             StrExpr,
	             CharExpr,
	             VarExpr,
	             UserInputExpr,
	             FuncCallExpr,
	             LenCallExpr,
	             PositionCallExpr,
	             SubStrCallExpr,
	             StrToIntCallExpr,
	             StrToRealCallExpr,
	             IntToStrCallExpr,
	             RealToStrCallExpr,
	             CharToCodeCallExpr,
	             CodeToCharCallExpr,
	             RandomIntCallExpr,
	             ListAccessExpr,
	             FieldAccessExpr,
	             ObjectCreationExpr> m_atom{}; /* The atom expression variant */
};

/* Struct representing a binary operation expression */
struct BinOpExpr
{
	std::shared_ptr<Expr> m_lhs{}; /* The lhs of the binary expression */
	std::shared_ptr<Expr> m_rhs{}; /* The rhs of the binary expression */
	Operator m_op{}; /* The binary operator */
};

/* Struct representing a unary operation expression */
struct UnaryOpExpr
{
	std::shared_ptr<Expr> m_unary_expr{}; /* The expression to apply the unary operator to */
	Operator m_op{}; /* The unary operator */
};

/* Struct representing a parenthesized expression */
struct ParenExpr
{
	std::shared_ptr<Expr> m_expr{}; /* The expression inside parentheses */
};

/* Struct representing a list element */
struct Element
{
	std::shared_ptr<Expr> m_expr{}; /* The expression held by the element */
};

/* Struct representing a list of elements */
struct List
{
	std::vector<Element> m_list{}; /* The list of elements */
};

/* Struct representing a list of expressions */
struct ListExpr
{
	List m_list{}; /* The list of expressions */
};

/* Struct representing a general expression */
struct Expr
{
	std::variant<AtomExpr,
	             BinOpExpr,
	             UnaryOpExpr,
	             ParenExpr,
	             ListExpr> m_expr{}; /* The expression variant */

	DataType m_type{}; /* The resolved data type of the expression */
};

/* Struct representing the abstract syntax tree */
struct AST
{
	Body m_body{}; /* The body of the program */
};

#endif