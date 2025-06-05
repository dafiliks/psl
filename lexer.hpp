#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include "error.hpp"

enum class Token_Type {
	IDENTIFIER,
	/* types */
	INT_LIT,
	FLOAT,
	STRING_LIT,
	/* single char tokens */
	GREATER_THAN,
	LESS_THAN,
	EQUALS,
	EXCLAIMATION,
	DASH,
	SQ_O_BRACKET,
	SQ_C_BRACKET,
	O_PAREN,
	C_PAREN,
	PLUS,
	MINUS,
	MULTIPLY,
	DIVIDE,
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
	{"=", Token_Type::EQUALS},
	{"!", Token_Type::EXCLAIMATION},
	{"-", Token_Type::DASH},
	{"[", Token_Type::SQ_O_BRACKET},
	{"]", Token_Type::SQ_C_BRACKET},
	{"(", Token_Type::O_PAREN},
	{")", Token_Type::C_PAREN},
	{"+", Token_Type::PLUS},
	{"-", Token_Type::MINUS},
	{"*", Token_Type::MULTIPLY},
	{"/", Token_Type::DIVIDE},
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

class Lexer {
public:
	Lexer(int argc, char** argv);
	~Lexer() = default;

	std::vector<Token> lex();
	char peek(std::size_t dist = 0);
	char eat();
	void validate_argc_argv(int argc, char** argv);
	std::string source_to_string(char* file_name);
	bool is_separator(char chr) const;
	bool find_token_vt_map(std::string value);
	void lex_ident_or_kw();
	void lex_number();
	void lex_string_lit();
	void lex_comment();

	std::vector<Token> get_tokens() const;
	std::string get_file_name() const;
	std::string get_source() const;
    Error get_lex_error() const;
private:
    Error m_lex_error{};
	std::string m_source{};
	std::string m_file_name{};
	std::vector<Token> m_tokens{};
	std::size_t m_index{};
	std::string m_buffer{};
	std::size_t m_line{1};
	std::size_t m_col{1};
};

#endif
