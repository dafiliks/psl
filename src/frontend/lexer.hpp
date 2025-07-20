#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include <string>
#include <unordered_map>

#include "../utils/error.hpp"

enum class Token_Type {
	IDENTIFIER,
	/* types */
	INT_LIT,
	FLOAT,
	STRING_LIT,
	/* single char tokens */
	GREATER_THAN,
	LESS_THAN,
	UNDERSCORE,
	EQUALS,
	EXCLAIMATION,
	SQ_O_BRACKET,
	SQ_C_BRACKET,
	O_PAREN,
	C_PAREN,
	PLUS,
	MINUS,
	MULTIPLY,
	DIVIDE,
	COMMA,
	/* keywords */
	CONSTANT,
	DIV,
	MOD,
	AND,
	OR,
	NOT,
	REPEAT,
	UNTIL,
	WHILE,
	END_WHILE,
	FOR,
	TO,
	END_FOR,
	IF,
	THEN,
	ELSE,
	END_IF,
	SUB_ROUTINE,
	END_SUB_ROUTINE,
	RETURN,
	USER_INPUT,
	OUTPUT,
	/* extra */
	END_OF_FILE,
};

static const std::unordered_map<std::string, Token_Type> value_token_map {
	/* single char tokens */
	{">", Token_Type::GREATER_THAN},
	{"<", Token_Type::LESS_THAN},
	{"_", Token_Type::UNDERSCORE},
	{"=", Token_Type::EQUALS},
	{"!", Token_Type::EXCLAIMATION},
	{"[", Token_Type::SQ_O_BRACKET},
	{"]", Token_Type::SQ_C_BRACKET},
	{"(", Token_Type::O_PAREN},
	{")", Token_Type::C_PAREN},
	{"+", Token_Type::PLUS},
	{"-", Token_Type::MINUS},
	{"*", Token_Type::MULTIPLY},
	{"/", Token_Type::DIVIDE},
	{",", Token_Type::COMMA},
	/* keywords */
	{"CONSTANT", Token_Type::CONSTANT},
	{"DIV", Token_Type::DIV},
	{"MOD", Token_Type::MOD},
	{"AND", Token_Type::AND},
	{"OR" , Token_Type::OR},
	{"NOT", Token_Type::NOT},
	{"REPEAT", Token_Type::REPEAT},
	{"UNTIL", Token_Type::UNTIL},
	{"WHILE", Token_Type::WHILE},
	{"ENDWHILE", Token_Type::END_WHILE},
	{"FOR", Token_Type::FOR},
	{"TO",  Token_Type::TO},
	{"ENDFOR", Token_Type::END_FOR},
	{"IF", Token_Type::IF},
	{"THEN", Token_Type::THEN},
	{"ELSE", Token_Type::ELSE},
	{"ENDIF", Token_Type::END_IF},
	{"SUBROUTINE", Token_Type::SUB_ROUTINE},
	{"ENDSUBROUTINE", Token_Type::END_SUB_ROUTINE},
	{"RETURN", Token_Type::RETURN},
	{"USERINPUT", Token_Type::USER_INPUT},
	{"OUTPUT", Token_Type::OUTPUT}
};

std::string to_string(Token_Type type);

struct Token {
	Token_Type m_type{};
	std::string m_value{};
	std::size_t m_line{};
	std::size_t m_col{};
};

#define lex_error_s(a, b, c) Error{a, b, c, m_source}
#define lex_error_l(a) Error{a}

class Lexer {
public:
	Lexer() = default;
	Lexer(const std::string& source);

	void lex();

	[[nodiscard]] const std::vector<Token>& get_tokens() const;
	[[nodiscard]] const std::string& get_source() const;

private:
	[[nodiscard]] bool find_token_vt_map(const std::string& value);
	void lex_ident_or_kw();
	void lex_number();
	void lex_string_lit();
	void lex_comment();

	[[nodiscard]] bool is_separator(char character) const;
	[[nodiscard]] bool is_decimal(char character) const;

	[[nodiscard]] char peek(std::size_t distance = 0);
	char eat();

	// private members
	std::vector<Token> m_tokens{};
	std::string m_source{};
	std::size_t m_index{};
	std::string m_buffer{};
	std::size_t m_line{1};
	std::size_t m_col{1};
};

#endif
