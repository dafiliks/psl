/* frontend/lexer.cpp by David Filiks */
/* The lexer implementation for the PsL compiler */

#include "lexer.hpp"

std::string_view tt_to_string(const TokenType type)
{
    /* Switch through all of the possible token types */
    switch (type)
    {
        /* If the token is an IDENTIFIER */
        case TokenType::IDENTIFIER:
            /* Return string representation */
            return "identifier";

        /* If the token is an INT */
        case TokenType::INT:
            /* Return string representation */
            return "integer";

        /* If the token is a REAL */
        case TokenType::REAL:
            /* Return string representation */
            return "real";

        /* If the token is a STRING */
        case TokenType::STRING:
            /* Return string representation */
            return "string";

        /* If the token is a CHAR */
        case TokenType::CHAR:
            /* Return string representation */
            return "char";

        /* If the token is GREATER_THAN */
        case TokenType::GREATER_THAN:
            /* Return string representation */
            return ">";

        /* If the token is LESS_THAN */
        case TokenType::LESS_THAN:
            /* Return string representation */
            return "<";

        /* If the token is UNDERSCORE */
        case TokenType::UNDERSCORE:
            /* Return string representation */
            return "_";

        /* If the token is EQUALS */
        case TokenType::EQUALS:
            /* Return string representation */
            return "=";

        /* If the token is EXCLAMATION */
        case TokenType::EXCLAMATION:
            /* Return string representation */
            return "!";

        /* If the token is SQ_O_BRACKET */
        case TokenType::SQ_O_BRACKET:
            /* Return string representation */
            return "[";

        /* If the token is SQ_C_BRACKET */
        case TokenType::SQ_C_BRACKET:
            /* Return string representation */
            return "]";

        /* If the token is O_PAREN */
        case TokenType::O_PAREN:
            /* Return string representation */
            return "(";

        /* If the token is C_PAREN */
        case TokenType::C_PAREN:
            /* Return string representation */
            return ")";

        /* If the token is ADDITION */
        case TokenType::ADDITION:
            /* Return string representation */
            return "+";

        /* If the token is SUBTRACTION */
        case TokenType::SUBTRACTION:
            /* Return string representation */
            return "-";

        /* If the token is MULTIPLICATION */
        case TokenType::MULTIPLICATION:
            /* Return string representation */
            return "*";

        /* If the token is DIVISION */
        case TokenType::DIVISION:
            /* Return string representation */
            return "/";

        /* If the token is COMMA */
        case TokenType::COMMA:
            /* Return string representation */
            return ",";

        /* If the token is DOT */
        case TokenType::DOT:
            /* Return string representation */
            return ".";

        /* If the token is COLON */
        case TokenType::COLON:
            /* Return string representation */
            return ":";

        /* If the token is CONSTANT */
        case TokenType::CONSTANT:
            /* Return string representation */
            return "CONSTANT";

        /* If the token is DIV */
        case TokenType::DIV:
            /* Return string representation */
            return "DIV";

        /* If the token is MOD */
        case TokenType::MOD:
            /* Return string representation */
            return "MOD";

        /* If the token is AND */
        case TokenType::AND:
            /* Return string representation */
            return "AND";

        /* If the token is OR */
        case TokenType::OR:
            /* Return string representation */
            return "OR";

        /* If the token is NOT */
        case TokenType::NOT:
            /* Return string representation */
            return "NOT";

        /* If the token is REPEAT */
        case TokenType::REPEAT:
            /* Return string representation */
            return "REPEAT";

        /* If the token is UNTIL */
        case TokenType::UNTIL:
            /* Return string representation */
            return "UNTIL";

        /* If the token is WHILE */
        case TokenType::WHILE:
            /* Return string representation */
            return "WHILE";

        /* If the token is END_WHILE */
        case TokenType::END_WHILE:
            /* Return string representation */
            return "ENDWHILE";

        /* If the token is FOR */
        case TokenType::FOR:
            /* Return string representation */
            return "FOR";

        /* If the token is TO */
        case TokenType::TO:
            /* Return string representation */
            return "TO";

        /* If the token is IN */
        case TokenType::IN:
            /* Return string representation */
            return "IN";

        /* If the token is STEP */
        case TokenType::STEP:
            /* Return string representation */
            return "STEP";

        /* If the token is END_FOR */
        case TokenType::END_FOR:
            /* Return string representation */
            return "ENDFOR";

        /* If the token is IF */
        case TokenType::IF:
            /* Return string representation */
            return "IF";

        /* If the token is THEN */
        case TokenType::THEN:
            /* Return string representation */
            return "THEN";

        /* If the token is ELSE */
        case TokenType::ELSE:
            /* Return string representation */
            return "ELSE";

        /* If the token is END_IF */
        case TokenType::END_IF:
            /* Return string representation */
            return "ENDIF";

        /* If the token is RECORD */
        case TokenType::RECORD:
            /* Return string representation */
            return "RECORD";

        /* If the token is END_RECORD */
        case TokenType::END_RECORD:
            /* Return string representation */
            return "ENDRECORD";

        /* If the token is SUB_ROUTINE */
        case TokenType::SUB_ROUTINE:
            /* Return string representation */
            return "SUBROUTINE";

        /* If the token is RETURN */
        case TokenType::RETURN:
            /* Return string representation */
            return "RETURN";

        /* If the token is END_SUB_ROUTINE */
        case TokenType::END_SUB_ROUTINE:
            /* Return string representation */
            return "ENDSUBROUTINE";

        /* If the token is USER_INPUT */
        case TokenType::USER_INPUT:
            /* Return string representation */
            return "USERINPUT";

        /* If the token is STRING_TYPE */
        case TokenType::STRING_TYPE:
            /* Return string representation */
            return "String";

        /* If the token is REAL_TYPE */
        case TokenType::REAL_TYPE:
            /* Return string representation */
            return "Real";

        /* If the token is INT_TYPE */
        case TokenType::INT_TYPE:
            /* Return string representation */
            return "Integer";

        /* If the token is CHAR_TYPE */
        case TokenType::CHAR_TYPE:
            /* Return string representation */
            return "Char";

        /* If the token is LEN */
        case TokenType::LEN:
            /* Return string representation */
            return "LEN";

        /* If the token is POSITION */
        case TokenType::POSITION:
            /* Return string representation */
            return "POSITION";

        /* If the token is SUBSTRING */
        case TokenType::SUBSTRING:
            /* Return string representation */
            return "SUBSTRING";

        /* If the token is STRING_TO_INT */
        case TokenType::STRING_TO_INT:
            /* Return string representation */
            return "STRING_TO_INT";

        /* If the token is STRING_TO_REAL */
        case TokenType::STRING_TO_REAL:
            /* Return string representation */
            return "STRING_TO_REAL";

        /* If the token is INT_TO_STRING */
        case TokenType::INT_TO_STRING:
            /* Return string representation */
            return "INT_TO_STRING";

        /* If the token is REAL_TO_STRING */
        case TokenType::REAL_TO_STRING:
            /* Return string representation */
            return "REAL_TO_STRING";

        /* If the token is CHAR_TO_CODE */
        case TokenType::CHAR_TO_CODE:
            /* Return string representation */
            return "CHAR_TO_CODE";

        /* If the token is CODE_TO_CHAR */
        case TokenType::CODE_TO_CHAR:
            /* Return string representation */
            return "CODE_TO_CHAR";

        /* If the token is OUTPUT */
        case TokenType::OUTPUT:
            /* Return string representation */
            return "OUTPUT";

        /* If the token is RANDOM_INT */
        case TokenType::RANDOM_INT:
            /* Return string representation */
            return "RANDOM_INT";

        /* If the token is END_OF_FILE */
        case TokenType::END_OF_FILE:
            /* Return string representation */
            return "eof";


        /* If no matches occur */
        default:
            /* Return unknown token type */
            return "unknown token type";
    }
}


Lexer::Lexer(const CLIArgs& args)
/* Initialize source related members */
: m_source(args.get_source() + '\0'),
  m_source_path(args.get_source_path()) {}

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
            /* Throw lex error */
            throw LexError
            {
                "no matching token found for \"" + std::string{peek()} + "\"",
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
    if (got)
    {
        /* Add the token value along with the token type to the token stream */
        m_tokens.push_back({value, *got, m_row, m_col});

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
    /* Consume the opening quote character in the string literal */
    consume();

    /* Loop while the string is not yet terminated */
    while (peek() != '\0' && peek() != '\'')
    {
        /* Consume the current character and append to the buffer */
        m_buffer += consume();
    }

    /* If the current character is the closing quote character */
    if (peek() == '\'')
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
    /* If the peek offset is out of range */
    if (m_index + distance >= m_source.size())
    {
        /* Throw lex error */
        throw LexError
        {
            "peek offset out of range",
            m_row,
            m_col,
            m_source
        };
    }

    /* Return the character present at the specified distance */
    return m_source[m_index + distance];
}

char Lexer::consume()
{
    /* If the consume offset is out of range */
    if (m_index + 1 >= m_source.size())
    {
        /* Throw lex error */
        throw LexError
        {
            "consume offset out of range",
            m_row,
            m_col,
            m_source
        };
    }

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