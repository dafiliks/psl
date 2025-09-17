/* utils/error.cpp by David Filiks */
/* The error implementation for the PsL compiler */

#include "error.hpp"

std::string_view et_to_string(const ErrorType type)
{
    /* Switch through all of the possible error types */
    switch (type)
    {
        /* If the error type is CLI_ARGS */
        case (ErrorType::CLI_ARGS):
            /* Return string representation */
            return "cli args";

        /* If the error type is LEX */
        case (ErrorType::LEX):
            /* Return string representation */
            return "lex";

        /* If the error type is PARSE */
        case (ErrorType::PARSE):
            /* Return string representation */
            return "parse";

        /* If the error type is STACK */
        case (ErrorType::STACK):
            /* Return string representation */
            return "stack";

        /* If the error type is GEN */
        case (ErrorType::GEN):
            /* Return string representation */
            return "gen";

        /* If the error type is COMPILE */
        case (ErrorType::COMPILE):
            /* Return string representation */
            return "compile";

        /* If no matches occur */
        default:
            /* Return unknown error type */
            return "unknown";
    }
}


Error::Error(const std::string_view message, const ErrorType type)
/* Initialize error type */
: m_type(type)
{
    /* Store appropriate error message */
    m_error << et_to_string(m_type) << " error: " <<  message << "\n";
}

Error::Error(const std::string_view message, const std::size_t row, const std::size_t col, const std::string& source, const ErrorType type)
/* Initialize members */
: m_source(std::move(source)),
  m_type(type)
{
    /* Store appropriate error message */
    m_error << et_to_string(m_type) << " error: " << row << ":" << col << ": " << message << "\n";

    /* Add the part of the source code which caused the error */
    m_error << add_source_error(row, col).str();
}

[[nodiscard]] const std::string& Error::get_source() const
{
    /* Return the source contents */
    return m_source;
}

[[nodiscard]] const ErrorType& Error::get_type() const
{
    /* Return error type */
    return m_type;
}

[[nodiscard]] std::string Error::what() const
{
    /* Return final error message */
    return m_error.str();
}

std::ostringstream Error::add_source_error(const std::size_t row, const std::size_t col) const
{
    std::ostringstream m_source_error{}; /* Holds the source error */

    std::size_t start{}; /* Source error start index */
    std::size_t end{}; /* Source error end index */

    /* Find the start index of the error */
    for (std::size_t i{}; i < row - 1; i++)
    {
        /* Move to next newline */
        start = m_source.find("\n", start);

        /* Consume the newline character */
        start++;
    }

    /* Find the end index of the error */
    end = m_source.find('\n', start);

    /* If no newline found */
    if (end == std::string::npos)
    {
        /* Search for an end of file */
        end = m_source.find('\0', start);
    }

    /* Add the row number next to the source error */
    m_source_error << row << " | ";

    /* Add the entire line containing the error to the source error */
    m_source_error << m_source.substr(start, end - start) << "\n";

    /* Return the source error stream */
    return m_source_error;
}
