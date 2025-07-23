#include <vector>
#include <cctype>
#include <fstream>
#include <cstring>
#include <cassert>
#include <iostream>

#include "lexer.hpp"
#include "../utils/error.hpp"

[[nodiscard]] std::string to_string(Token_Type type)
{
	switch (type) {
		case Token_Type::IDENTIFIER:         return "IDENTIFIER";
		case Token_Type::FLOAT:              return "FLOAT";
		case Token_Type::STRING_LIT:         return "STRING_LIT";
		case Token_Type::GREATER_THAN:       return "GREATER_THAN";
		case Token_Type::LESS_THAN:          return "LESS_THAN";
		case Token_Type::UNDERSCORE:         return "_";
		case Token_Type::EQUALS:             return "EQUALS";
		case Token_Type::EXCLAIMATION:       return "EXCLAIMATION";
		case Token_Type::SQ_O_BRACKET:       return "SQ_O_BRACKET";
		case Token_Type::SQ_C_BRACKET:       return "SQ_C_BRACKET";
		case Token_Type::O_PAREN:            return "O_PAREN";
		case Token_Type::C_PAREN:            return "C_PAREN";
		case Token_Type::PLUS:               return "+";
		case Token_Type::MINUS:              return "-";
		case Token_Type::MULTIPLY:           return "*";
		case Token_Type::DIVIDE:             return "/";
		case Token_Type::COMMA:              return ",";
		case Token_Type::DIV:                return "//";
		case Token_Type::MOD:                return "%";
		case Token_Type::AND:                return "AND";
		case Token_Type::OR:                 return "OR";
		case Token_Type::NOT:                return "NOT";
		case Token_Type::REPEAT:             return "REPEAT";
		case Token_Type::UNTIL:              return "UNTIL";
		case Token_Type::WHILE:              return "WHILE";
		case Token_Type::END_WHILE:          return "END_WHILE";
		case Token_Type::FOR:                return "FOR";
		case Token_Type::TO:                 return "TO";
		case Token_Type::END_FOR:            return "END_FOR";
		case Token_Type::IF:                 return "IF";
		case Token_Type::THEN:               return "THEN";
		case Token_Type::ELSE:               return "ELSE";
		case Token_Type::END_IF:             return "END_IF";
		case Token_Type::SUB_ROUTINE:        return "SUB_ROUTINE";
		case Token_Type::END_SUB_ROUTINE:    return "END_SUB_ROUTINE";
		case Token_Type::RETURN:             return "RETURN";
		case Token_Type::USER_INPUT:         return "USER_INPUT";
		case Token_Type::OUTPUT:             return "OUTPUT";
		case Token_Type::END_OF_FILE:        return "END_OF_FILE";
		default:                             return "UNKNOWN_TOKEN_TYPE";
	}
}

Lexer::Lexer(const std::string& source) : m_source(source + '\0') {}

void Lexer::lex()
{
	while (peek() != '\0') {
		if (find_token_vt_map(std::string{peek()})) {
			eat();
		} else if (isalpha(peek())) {
			lex_ident_or_kw();
		} else if (isdigit(peek())) {
			lex_number();
		} else if (peek() == '"') {
			lex_string_lit();
		} else if (peek() == '#') {
			lex_comment();
		} else if (is_separator(peek())) {
			eat();
		} else {
			lex_error_s(
				"no matching token found for '" + std::string{peek()} + "'",
				m_line,
				m_col);
		}
	}

	m_tokens.push_back({Token_Type::END_OF_FILE, "", m_line, m_col});
	m_buffer.clear();
}

[[nodiscard]] const std::vector<Token>& Lexer::get_tokens() const { return m_tokens; }
[[nodiscard]] const std::string& Lexer::get_source()        const { return m_source; }

[[nodiscard]] bool Lexer::find_token_vt_map(const std::string& value)
{
	auto got{value_token_map.find(value)};
	if (got != value_token_map.end()) {
		m_tokens.push_back({got->second, got->first, m_line, m_col});
		m_buffer.clear();
		return true;
	}

	return false;
}

void Lexer::lex_ident_or_kw()
{
	do {
		m_buffer += eat();
	} while (!is_separator(peek()) && isalnum(peek()) || peek() == '_');

	if (!find_token_vt_map(m_buffer)) {
		m_tokens.push_back({Token_Type::IDENTIFIER, m_buffer, m_line, m_col});
		m_buffer.clear();
	}
}

void Lexer::lex_number()
{
	do {
		m_buffer += eat();
	} while (!is_separator(peek()) && is_decimal(peek()));

	m_tokens.push_back({Token_Type::FLOAT, m_buffer, m_line, m_col});
	m_buffer.clear();
}

void Lexer::lex_string_lit()
{
	m_buffer += eat();
	do {
		m_buffer += eat();
	} while (peek() != '\0' && peek() != '"');

	m_buffer += eat();
	m_tokens.push_back({Token_Type::STRING_LIT, m_buffer, m_line, m_col});
	m_buffer.clear();
}

void Lexer::lex_comment()
{
	do {
		eat();
	} while (peek() != '\0' && peek() != '\n');
}

[[nodiscard]] bool Lexer::is_separator(char character) const
{
	return character == ' '  ||
	       character == '\n' ||
	       character == '\t' ||
	       character == '\0';
}

[[nodiscard]] bool Lexer::is_decimal(char character) const { return isdigit(character) || character == '.'; }

[[nodiscard]] char Lexer::peek(std::size_t distance)
{
	assert(m_index + distance <= m_source.size());
	return m_source.at(m_index + distance);
}

char Lexer::eat()
{
	assert(m_index + 1 <= m_source.size());
	m_index++;
	if (peek() == '\n') {
		m_line++;
		m_col = 0;
	} else {
		m_col++;
	}
	return peek(-1);
}
