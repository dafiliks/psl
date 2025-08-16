/* frontend/lexer.hpp by David Filiks */
/* The lexer header for the PsL compiler */

#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <cstring>
#include <cassert>

#include "../utils/cliargs.hpp"
#include "../compiler/compilation_stage.hpp"
#include "../utils/error_types.hpp"

/* Enum class of all the token types used in the language */
enum class TokenType
{
	/* Simple language constructs */

	IDENTIFIER, /* Represents non-keyword names of functions/variables (e.g. pi) */
	INT, /* Represents whole numbers (e.g. 10) */
	REAL, /* Represents floating point numbers (e.g. 3.14) */
	STRING, /* Represents a sequence of characters enclosed by '' (e.g. 'message') */
	CHAR, /* Represents a single character enclosed by '' (e.g. 'a') */

	/* Single character tokens */

	GREATER_THAN, /* Represents the greater than ">" character */
	LESS_THAN, /* Represents the less than "<" character */
	UNDERSCORE, /* Represents the underscore "_" character */
	EQUALS, /* Represents the equals "=" character */
	EXCLAMATION, /* Represents the exclamation mark "!" character */
	SQ_O_BRACKET, /* Represents the square open bracket "[" character */
	SQ_C_BRACKET, /* Represents the square close bracket "]" character */
	O_PAREN, /* Represents the open parenthesis "(" character */
	C_PAREN, /* Represents the close parenthesis ")" character */
	ADDITION, /* Represents the addition "+" character */
	SUBTRACTION, /* Represents the subtraction "-" character */
	MULTIPLICATION, /* Represents the multiplication "*" character */
	DIVISION, /* Represents the division "/" character */
	COMMA, /* Represents the comma "," character */
	DOT, /* Represents the full stop "." character */
	COLON, /* Represents the colon ":" character */

	/* Keywords */

	CONSTANT, /* Represents the "CONSTANT" keyword */
	DIV, /* Represents the "DIV" keyword */
	MOD, /* Represents the "MOD" keyword */
	AND, /* Represents the "AND" keyword */
	OR, /* Represents the "OR" keyword */
	NOT, /* Represents the "NOT" keyword */
	REPEAT, /* Represents the "REPEAT" keyword */
	UNTIL, /* Represents the "UNTIL" keyword */
	WHILE, /* Represents the "WHILE" keyword */
	END_WHILE, /* Represents the "ENDWHILE" keyword */
	FOR, /* Represents the "FOR" keyword */
	TO, /* Represents the "TO" keyword */
	IN, /* Represents the "IN" keyword */
	STEP, /* Represents the "STEP" keyword */
	END_FOR, /* Represents the "ENDFOR" keyword */
	IF, /* Represents the "IF" keyword */
	THEN, /* Represents the "THEN" keyword */
	ELSE, /* Represents the "ELSE" keyword */
	END_IF, /* Represents the "ENDIF" keyword */
	RECORD, /* Represents the "RECORD" keyword */
	END_RECORD, /* Represents the "ENDRECORD" keyword */
	SUB_ROUTINE, /* Represents the "SUBROUTINE" keyword */
	RETURN, /* Represents the "RETURN" keyword */
	END_SUB_ROUTINE, /* Represents the "ENDSUBROUTINE" keyword */
	USER_INPUT, /* Represents the "USERINPUT" keyword */

	/* Explicit field data type keywords */

	STRING_TYPE, /* Represents the "String" data type keyword */
	REAL_TYPE, /* Represents the "Real" data type keyword */
	INT_TYPE, /* Represents the "Integer" data type keyword */
	CHAR_TYPE, /* Represents the "Char" data type keyword */

	/* Standard library functions */

	LEN, /* Represents the "LEN()" standard library function call */
	POSITION, /* Represents the "POSITION()" standard library function call */
	SUBSTRING, /* Represents the "SUBSTRING()" standard library function call */
	STRING_TO_INT, /* Represents the "STRING_TO_INT()" standard library function call */
	STRING_TO_REAL, /* Represents the "STRING_TO_REAL()" standard library function call */
	INT_TO_STRING, /* Represents the "INT_TO_STRING()" standard library function call */
	REAL_TO_STRING, /* Represents the "REAL_TO_STRING()" standard library function call */
	CHAR_TO_CODE, /* Represents the "CHAR_TO_CODE()" standard library function call */
	CODE_TO_CHAR, /* Represents the "CODE_TO_CHAR()" standard library function call */
	OUTPUT, /* Represents the "OUTPUT" standard library function call */
	RANDOM_INT, /* Represents the "RANDOM_INT()" standard library function call */

	/* Extra useful tokens */

	END_OF_FILE, /* Represents the null "\0" character */
};

/* A map between known symbols, identifiers, and keywords and their TokenType equivalent */
static const std::unordered_map<std::string, TokenType> value_token_map
{
	/* Single character tokens */

	{">", TokenType::GREATER_THAN}, /* Maps ">" to TokenType::GREATER_THAN */
	{"<", TokenType::LESS_THAN}, /* Maps "<" to TokenType::LESS_THAN */
	{"_", TokenType::UNDERSCORE}, /* Maps "_" to TokenType::UNDERSCORE */
	{"=", TokenType::EQUALS}, /* Maps "=" to TokenType::EQUALS */
	{"!", TokenType::EXCLAMATION}, /* Maps "!" to TokenType::EXCLAMATION */
	{"[", TokenType::SQ_O_BRACKET}, /* Maps "[" to TokenType::SQ_O_BRACKET */
	{"]", TokenType::SQ_C_BRACKET}, /* Maps "]" to TokenType::SQ_C_BRACKET */
	{"(", TokenType::O_PAREN}, /* Maps "(" to TokenType::O_PAREN */
	{")", TokenType::C_PAREN}, /* Maps ")" to TokenType::C_PAREN */
	{"+", TokenType::ADDITION}, /* Maps "+" to TokenType::ADDITION */
	{"-", TokenType::SUBTRACTION}, /* Maps "-" to TokenType::SUBTRACTION */
	{"*", TokenType::MULTIPLICATION}, /* Maps "*" to TokenType::MULTIPLICATION */
	{"/", TokenType::DIVISION}, /* Maps "/" to TokenType::DIVISION */
	{",", TokenType::COMMA}, /* Maps "," to TokenType::COMMA */
	{".", TokenType::DOT}, /* Maps "." to TokenType::DOT */
	{":", TokenType::COLON}, /* Maps ":" to TokenType::COLON */

	/* Keywords */

	{"CONSTANT", TokenType::CONSTANT}, /* Maps "CONSTANT" to TokenType::CONSTANT */
	{"DIV", TokenType::DIV}, /* Maps "DIV" to TokenType::DIV */
	{"MOD", TokenType::MOD}, /* Maps "MOD" to TokenType::MOD */
	{"AND", TokenType::AND}, /* Maps "AND" to TokenType::AND */
	{"OR", TokenType::OR}, /* Maps "OR" to TokenType::OR */
	{"NOT", TokenType::NOT}, /* Maps "NOT" to TokenType::NOT */
	{"REPEAT", TokenType::REPEAT}, /* Maps "REPEAT" to TokenType::REPEAT */
	{"UNTIL", TokenType::UNTIL}, /* Maps "UNTIL" to TokenType::UNTIL */
	{"WHILE", TokenType::WHILE}, /* Maps "WHILE" to TokenType::WHILE */
	{"ENDWHILE", TokenType::END_WHILE}, /* Maps "ENDWHILE" to TokenType::END_WHILE */
	{"FOR", TokenType::FOR}, /* Maps "FOR" to TokenType::FOR */
	{"TO", TokenType::TO}, /* Maps "TO" to TokenType::TO */
	{"IN", TokenType::IN}, /* Maps "IN" to TokenType::IN */
	{"STEP", TokenType::STEP}, /* Maps "STEP" to TokenType::STEP */
	{"ENDFOR", TokenType::END_FOR}, /* Maps "ENDFOR" to TokenType::END_FOR */
	{"IF", TokenType::IF}, /* Maps "IF" to TokenType::IF */
	{"THEN", TokenType::THEN}, /* Maps "THEN" to TokenType::THEN */
	{"ELSE", TokenType::ELSE}, /* Maps "ELSE" to TokenType::ELSE */
	{"ENDIF", TokenType::END_IF}, /* Maps "ENDIF" to TokenType::END_IF */
	{"RECORD", TokenType::RECORD}, /* Maps "RECORD" to TokenType::RECORD */
	{"ENDRECORD", TokenType::END_RECORD}, /* Maps "ENDRECORD" to TokenType::END_RECORD */
	{"SUBROUTINE", TokenType::SUB_ROUTINE}, /* Maps "SUBROUTINE" to TokenType::SUB_ROUTINE */
	{"RETURN", TokenType::RETURN}, /* Maps "RETURN" to TokenType::RETURN */
	{"ENDSUBROUTINE", TokenType::END_SUB_ROUTINE}, /* Maps "ENDSUBROUTINE" to TokenType::END_SUB_ROUTINE */
	{"USERINPUT", TokenType::USER_INPUT}, /* Maps "USERINPUT" to TokenType::USER_INPUT */

	/* Explicit field data type keywords */

	{"String", TokenType::STRING_TYPE}, /* Maps "String" to TokenType::STRING_TYPE */
	{"Real", TokenType::REAL_TYPE}, /* Maps "Real" to TokenType::REAL_TYPE */
	{"Integer", TokenType::INT_TYPE}, /* Maps "Integer" to TokenType::INT_TYPE */
	{"Char", TokenType::CHAR_TYPE}, /* Maps "Char" to TokenType::CHAR_TYPE */

	/* Standard library functions */

	{"LEN", TokenType::LEN}, /* Maps "LEN" to TokenType::LEN */
	{"POSITION", TokenType::POSITION}, /* Maps "POSITION" to TokenType::POSITION */
	{"SUBSTRING", TokenType::SUBSTRING}, /* Maps "SUBSTRING" to TokenType::SUBSTRING */
	{"STRING_TO_INT", TokenType::STRING_TO_INT}, /* Maps "STRING_TO_INT" to TokenType::STRING_TO_INT */
	{"STRING_TO_REAL", TokenType::STRING_TO_REAL}, /* Maps "STRING_TO_REAL" to TokenType::STRING_TO_REAL */
	{"INT_TO_STRING", TokenType::INT_TO_STRING}, /* Maps "INT_TO_STRING" to TokenType::INT_TO_STRING */
	{"REAL_TO_STRING", TokenType::REAL_TO_STRING}, /* Maps "REAL_TO_STRING" to TokenType::REAL_TO_STRING */
	{"CHAR_TO_CODE", TokenType::CHAR_TO_CODE}, /* Maps "CHAR_TO_CODE" to TokenType::CHAR_TO_CODE */
	{"CODE_TO_CHAR", TokenType::CODE_TO_CHAR}, /* Maps "CODE_TO_CHAR" to TokenType::CODE_TO_CHAR */
	{"OUTPUT", TokenType::OUTPUT}, /* Maps "OUTPUT" to TokenType::OUTPUT */
	{"RANDOM_INT", TokenType::RANDOM_INT}, /* Maps "RANDOM_INT" to TokenType::RANDOM_INT */
};

/* Returns the corresponding string equivalent for a particular token type */
/* Param: const TokenType - the token type */
/* Returns: std::string_view - the string equivalent */
std::string_view tt_to_string(const TokenType type);

/* The struct used to represent a singular token */
struct Token
{
	std::string m_value{}; /* The string value of the token */
	TokenType m_type{};  /* The type of the token */

	std::size_t m_row{}; /* Row number of where the token occurs in the file */
	std::size_t m_col{}; /* Column number of where the token occurs in the file */
};

/* The main lexer class, responsible for converting the input source into a token stream */
/* Inherits from CompilationStage as lexical analysis is a compilation stage */
class Lexer : public CompilationStage
{

/* Public members */
public:

	/* Functions */

	/* Constructs a Lexer object */
	/* Param: const CLIArgs& - cli args passed to the program */
	Lexer(const CLIArgs& args);

	/* Executes the lex() function */
	void execute() override;

	/* Performs lexical analysis on the source */
	void lex();

	/* Getter function for the token stream */
	/* Returns: const std::vector<Token>& - token stream generated from input file */
	[[nodiscard]] const std::vector<Token>& get_tokens() const;

	/* Getter function for the source file contents */
	/* Returns: const std::string& - the source file contents */
	[[nodiscard]] const std::string& get_source() const;

	/* Getter function for the source file path */
	/* Returns: const std::string& - the source file path */
	[[nodiscard]] const std::string& get_source_path() const;

	/* Getter function for the source index */
	/* Returns: const std::size_t& - the current file index */
	[[nodiscard]] const std::size_t& get_index() const;

	/* Getter function for the token buffer */
	/* Returns: const std::string& - the contents of the token buffer */
	[[nodiscard]] const std::string& get_buffer() const;

	/* Getter function for the current source row position */
	/* Returns: const std::size_t& - the current source row position */
	[[nodiscard]] const std::size_t& get_row() const;

	/* Getter function for the current source col position */
	/* Returns: const std::size_t& - the current source col position */
	[[nodiscard]] const std::size_t& get_col() const;

/* Private members */
private:

	/* Functions */

	/* Checks if token value exists in the value token map */
	/* If it exists, the token stream is modified accordingly */
	/* Param: const std::string& - token value */
	/* Returns: bool - whether the token exists in the value token map */
	[[nodiscard]] bool find_token_vt_map(const std::string& value);

	/* Performs lexical analysis on an identifier or keyword */
	void lex_ident_or_kw();

	/* Performs lexical analysis on a number (integer or real) */
	void lex_number();

	/* Performs lexical analysis on a string sequence (string lit or char) */
	void lex_string_lit_or_char();

	/* Skips over characters appropriately in the case of a comment */
	void lex_comment();

	/* Checks if a character is a separator */
	/* Param: char - the character to check */
	/* Returns: bool - whether the character is a separator */
	[[nodiscard]] bool is_separator(char character) const;

	/* Peeks a certain distance away from the current file index */
	/* Param: const std::size_t - distance to peek */
	/* Returns: const char - character present at the peek position */
	[[nodiscard]] char peek(const std::size_t distance = 0) const;

	/* Consumes the current index character and adjusts the index accordingly */
	char consume();

	/* Variables */

	std::vector<Token> m_tokens{}; /* Token stream generated from input file */
	std::string m_source{}; /* The contents of the source file */
	std::string m_source_path{}; /* The path of the source file */

	std::size_t m_index{}; /* The current index position of the lexer in the source file */

	std::string m_buffer{}; /* Buffer used to intermediately store token values */

	std::size_t m_row{1}; /* Current row position of the index within the source file */
	std::size_t m_col{1}; /* Current col position of the index within the source file */
};

#endif
