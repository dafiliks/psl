/* frontend/lexer.cpp by David Filiks */
/* The lexer implementation for the PsL compiler */

#include "lexer.hpp"

std::string_view tt_to_string(const TokenType type)
{
	/* Switch through all of the possible token types */
	switch (type)
	{
		/* Simple language constructs */

		case TokenType::IDENTIFIER:         return "identifier";
		case TokenType::INT:                return "integer";
		case TokenType::REAL:               return "real";
		case TokenType::STRING:             return "string";
		case TokenType::CHAR:               return "char";

		/* Single character tokens */

		case TokenType::GREATER_THAN:       return ">";
		case TokenType::LESS_THAN:          return "<";
		case TokenType::UNDERSCORE:         return "_";
		case TokenType::EQUALS:             return "=";
		case TokenType::EXCLAMATION:        return "!";
		case TokenType::SQ_O_BRACKET:       return "[";
		case TokenType::SQ_C_BRACKET:       return "]";
		case TokenType::O_PAREN:            return "(";
		case TokenType::C_PAREN:            return ")";
		case TokenType::ADDITION:           return "+";
		case TokenType::SUBTRACTION:        return "-";
		case TokenType::MULTIPLICATION:     return "*";
		case TokenType::DIVISION:           return "/";
		case TokenType::COMMA:              return ",";
		case TokenType::DOT:                return ".";
		case TokenType::COLON:              return ":";

		/* Keywords */

		case TokenType::CONSTANT:           return "CONSTANT";
		case TokenType::DIV:                return "DIV";
		case TokenType::MOD:                return "MOD";
		case TokenType::AND:                return "AND";
		case TokenType::OR:                 return "OR";
		case TokenType::NOT:                return "NOT";
		case TokenType::REPEAT:             return "REPEAT";
		case TokenType::UNTIL:              return "UNTIL";
		case TokenType::WHILE:              return "WHILE";
		case TokenType::END_WHILE:          return "ENDWHILE";
		case TokenType::FOR:                return "FOR";
		case TokenType::TO:                 return "TO";
		case TokenType::IN:                 return "IN";
		case TokenType::STEP:               return "STEP";
		case TokenType::END_FOR:            return "ENDFOR";
		case TokenType::IF:                 return "IF";
		case TokenType::THEN:               return "THEN";
		case TokenType::ELSE:               return "ELSE";
		case TokenType::END_IF:             return "ENDIF";
		case TokenType::RECORD:             return "RECORD";
		case TokenType::END_RECORD:         return "ENDRECORD";
		case TokenType::SUB_ROUTINE:        return "SUBROUTINE";
		case TokenType::RETURN:             return "RETURN";
		case TokenType::END_SUB_ROUTINE:    return "ENDSUBROUTINE";
		case TokenType::USER_INPUT:         return "USERINPUT";

		/* Explicit field data type keywords */

		case TokenType::STRING_TYPE:        return "String";
		case TokenType::REAL_TYPE:          return "Real";
		case TokenType::INT_TYPE:           return "Integer";
		case TokenType::CHAR_TYPE:          return "Char";

		/* Standard library functions */

		case TokenType::LEN:                return "LEN";
		case TokenType::POSITION:           return "POSITION";
		case TokenType::SUBSTRING:          return "SUBSTRING";
		case TokenType::STRING_TO_INT:      return "STRING_TO_INT";
		case TokenType::STRING_TO_REAL:     return "STRING_TO_REAL";
		case TokenType::INT_TO_STRING:      return "INT_TO_STRING";
		case TokenType::REAL_TO_STRING:     return "REAL_TO_STRING";
		case TokenType::CHAR_TO_CODE:       return "CHAR_TO_CODE";
		case TokenType::CODE_TO_CHAR:       return "CODE_TO_CHAR";
		case TokenType::OUTPUT:             return "OUTPUT";
		case TokenType::RANDOM_INT:         return "RANDOM_INT";

		/* Extra useful tokens */

		case TokenType::END_OF_FILE:        return "eof";

		/* Unknown token type */

		default:                            return "unknown token type";
	}
}

Lexer::Lexer(const CLIArgs& args)
: m_source(args.get_source() + '\0'), m_source_path(args.get_source_path()) /* Initialize source string */ {}

void Lexer::execute()
{
	/* Execute the lex() function */
	lex();
}

void Lexer::lex()
{
	/* Loop until the current character signifies end of file */
	while (peek() != '\0')
	{
		/* If the current character is a single character token */
		/* Add it to the token stream accordingly */
		if (find_token_vt_map(std::string{peek()}))
		{
			/* Consume the single character token */
			consume();
		}

		/* If the current character is alphanumerical */
		else if (isalpha(peek()))
		{
			/* Lex the following characters as an identifier or keyword */
			lex_ident_or_kw();
		}

		/* If the current character is a digit */
		else if (isdigit(peek()))
		{
			/* Lex the following characters as a number */
			lex_number();
		}

		/* If the current character is a single quote */
		else if (peek() == '\'')
		{
			/* Lex the following characters as a string sequence */
			lex_string_lit_or_char();
		}

		/* If the current character is a hashtag, signifying a comment */
		else if (peek() == '#')
		{
			/* Skip over characters accordingly */
			lex_comment();
		}

		/* If the current character is a separator, such as a space */
		else if (is_separator(peek()))
		{
			/* Consume the current separator character */
			consume();
		}

		/* In the case that no matching token can be found */
		else
		{
			/* Error out */
			LexError
			{
				"no matching token found for '" + std::string{peek()} + "'",
				m_row,
				m_col,
				m_source
			};
		}
	}

	/* Add the end of file token - this allows for easier parsing */
	m_tokens.push_back({"", TokenType::END_OF_FILE, m_row, m_col});

	/* Clear the token buffer */
	m_buffer.clear();
}

[[nodiscard]] const std::vector<Token>& Lexer::get_tokens() const
{
	return m_tokens; /* Return the token stream */
}

[[nodiscard]] const std::string& Lexer::get_source() const
{
	return m_source; /* Return the source contents */
}

[[nodiscard]] const std::string& Lexer::get_source_path() const
{
	return m_source_path; /* Return the source path */
}

[[nodiscard]] const std::size_t& Lexer::get_index() const
{
	return m_index; /* Return the current source index */
}

[[nodiscard]] const std::string& Lexer::get_buffer() const
{
	return m_buffer; /* Return the current token buffer */
}

[[nodiscard]] const std::size_t& Lexer::get_row() const
{
	return m_row; /* Return the current source row index */
}

[[nodiscard]] const std::size_t& Lexer::get_col() const
{
	return m_col; /* Return the current source col index */
}

[[nodiscard]] bool Lexer::find_token_vt_map(const std::string& value)
{
	/* Search for the token in the value token map */
	auto got{value_token_map.find(value)};

	/* If the token was found in the map */
	if (got != value_token_map.end())
	{
		/* Add the token along with the token type to the token stream */
		m_tokens.push_back({got->first, got->second, m_row, m_col});

		/* Clear the token buffer */
		m_buffer.clear();

		/* Return true to signify found */
		return true;
	}

	/* If the character was not found, return false to signify not found */
	return false;
}

void Lexer::lex_ident_or_kw()
{
	/* Do until the current character does not imply it is part of the identifier */
	do
	{
		/* Concatenate identifier characters into the token buffer */
		m_buffer += consume();
	} while (!is_separator(peek()) && isalnum(peek()) || peek() == '_'); /* Loop condition */

	/* If the identifier is not a keyword */
	if (!find_token_vt_map(m_buffer))
	{
		/* Append the identifier along with type IDENTIFIER onto the token stream */
		m_tokens.push_back({m_buffer, TokenType::IDENTIFIER, m_row, m_col});

		/* Clear the token buffer */
		m_buffer.clear();
	}
}

void Lexer::lex_number()
{
	/* Variable to keep track of if the number is an integer or real */
	bool is_real{false};

	/* Do until the current character does not imply it is part of the number */
	do
	{
		/* Concatenate number characters into the token buffer */
		m_buffer += consume();

		/* If the new current character is a full stop */
		if (peek() == '.')
		{
			/* Set the tracking variable accordingly */
			is_real = true;
		}

	} while (!is_separator(peek()) && isdigit(peek()) || peek() == '.'); /* Loop condition */

	/* If the number is real */
	if (is_real)
	{
		/* Append the number along with type REAL onto the token stream */
		m_tokens.push_back({m_buffer, TokenType::REAL, m_row, m_col});
	}

	/* If the number is an integer */
	else
	{
		/* Append the number along with type INT onto the token stream */
		m_tokens.push_back({m_buffer, TokenType::INT, m_row, m_col});
	}

	/* Clear the token buffer */
	m_buffer.clear();
}

void Lexer::lex_string_lit_or_char()
{
	/* Consume the opening "'" character in the string literal */
	consume();

	/* Do until the current character does not imply it is part of the string literal */
	do
	{
		/* Concatenate string literal characters into the token buffer */
		m_buffer += consume();
	} while (peek() != '\0' && peek() != '\''); /* Loop condition */

	/* If the current character signifies an end of file */
	if (peek() == '\0')
	{
		/* Error out */
		LexError
		{
			"closing quote could not be found for string literal",
			m_row,
			m_col,
			m_source
		};

	}

	/* If the current character is the closing "'" character */
	else if (peek() == '\'')
	{
		/* Consume the closing single quote */
		consume();
	}

	/* If the token buffer contains a single character */
	if (m_buffer.size() == 1)
	{
		/* Append the char along with type CHAR onto the token stream */
		m_tokens.push_back({m_buffer, TokenType::CHAR, m_row, m_col});
	}

	/* If the token buffer contains a string literal */
	else
	{
		/* Append the string literal along with type STRING onto the token stream */
		m_tokens.push_back({m_buffer, TokenType::STRING, m_row, m_col});
	}

	/* Clear the token buffer */
	m_buffer.clear();
}

void Lexer::lex_comment()
{
	/* Do until the current character does not imply it is part of the comment */
	do
	{
		/* Skip over the current character as it is in the comment */
		consume();
	} while (peek() != '\0' && peek() != '\n'); /* Loop condition */
}

[[nodiscard]] bool Lexer::is_separator(char character) const
{
	return character == ' ' ||
	       character == '\n' ||
	       character == '\t' ||
	       character == '\0'; /* Return whether the character is a separator character */
}

[[nodiscard]] char Lexer::peek(const std::size_t distance) const
{
	/* Check that the peek offset is not out of range */
	assert(m_index + distance <= m_source.size());

	/* Return the character present at the specified distance */
	return m_source[m_index + distance];
}

char Lexer::consume()
{
	/* Check that the consume offset is not out of range */
	assert(m_index + 1 <= m_source.size());

	/* Store the current character */
	char consumed{peek()};

	/* If the current character is a newline */
	if (peek() == '\n')
	{
		/* Advance the row count */
		m_row++;

		/* Reset the col count as the index is on a new row */
		m_col = 0;
	}

	/* If the current character is not a newline */
	else
	{
		/* Advance the col count */
		m_col++;
	}


	/* Advance the source index by one */
	m_index++;

	/* Return the character that was consumed */
	return consumed;
}