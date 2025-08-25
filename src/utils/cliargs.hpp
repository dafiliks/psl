/* utils/cliargs.hpp by David Filiks */
/* The cli args header for the PsL compiler */

#ifndef CLIARGS_HPP
#define CLIARGS_HPP

#include <string>
#include <filesystem>
#include <fstream>

#include "../utils/error_types.hpp"

class CLIArgs
{
/* Public members */
public:

	/* Functions */

	/* Constructs a CLIArgs object */
	/* Param: int - argument count */
	/* Param: char** - argument vector */
	CLIArgs(int argc, char** argv);

	/* Handles the CLI arguments and follows flags */
	void handle();

	/* Getter function for the argument count */
	/* Returns: int - the argument count */
	[[nodiscard]] int get_argc() const;

	/* Getter function for the argument vector */
	/* Returns: char** - the argument vector */
	[[nodiscard]] char** get_argv() const;

	/* Getter function for the source contents */
	/* Returns: const std::string& - the source contents */
	[[nodiscard]] const std::string& get_source() const;

	/* Extra Misc */

	/* Getter function for the source path */
	/* Returns: std::string - the source path */
	[[nodiscard]] std::string get_source_path() const;

	/* Getter function for the target output flag */
	/* Returns: std::string - the target output flag */
	[[nodiscard]] std::string get_target_output_flag() const;

/* Private members */
private:

	/* Functions */

	/* Validates the CLI arguments */
	void validate_args();

	/* Reads a source file into the source member variable */
	/* Param: const std::string& - path to the source file */
	void file_to_source(const std::string& path);

	/* Checks whether a particular output target exists and is valid */
	/* Param: const std::string_view - the output target */
	/* Returns: bool - whether the output target is valid */
	[[nodiscard]] bool is_valid_output_target(const std::string_view output_target);

	/* Variables */

	int m_argc{}; /* The argument count of the program */
	char** m_argv{}; /* The argument strings for the program */

	std::string m_source{}; /* The source contents */
};

#endif
