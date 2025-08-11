/* utils/cliargs.cpp by David Filiks */
/* The cli args implementation for the PsL compiler */

#include <filesystem>
#include <fstream>

#include "../utils/cliargs.hpp"
#include "../utils/error_types.hpp"

CLIArgs::CLIArgs(int argc, char **argv) : m_argc(argc), m_argv(argv) /* Initializes members, argc and argv */ {}

void CLIArgs::handle()
{
	/* Validate the CLI arguments */
	validate_args();

	/* Store the source file contents as a string */
	file_to_source(get_source_path());
}

[[nodiscard]] int CLIArgs::get_argc() const
{
	return m_argc; /* Return argc (the argument count) */
}

[[nodiscard]] char** CLIArgs::get_argv() const
{
	return m_argv; /* Return argv (the arguemnt strings) */
}

[[nodiscard]] const std::string& CLIArgs::get_source() const
{
	return m_source; /* Return the source contents */
}

[[nodiscard]] std::string CLIArgs::get_source_path() const
{
	return std::string{m_argv[1]}; /* Return string copy of the source path */
}

[[nodiscard]] std::string CLIArgs::get_target_output_flag() const
{
	return std::string{m_argv[2]}; /* Return string copy of the target output flag */
}

void CLIArgs::validate_args()
{
	/* If the argument count is not equal to two */
	if (m_argc != 2)
	{
		/* Error out */
		CLIArgsError
		{
			"wrong number of arguments provided"
		};
	}

	/* If the file does not exist */
	if (!std::filesystem::exists(get_source_path()))
	{
		/* Error out */
		CLIArgsError
		{
			"file '" + get_source_path() + "' does not exist"
		};

	}

	/* If the file extension is not ".pseudo" */
	if (std::filesystem::path(get_source_path()).extension() != ".pseudo")
	{
		/* Error out */
		CLIArgsError
		{
			"file '" + get_source_path() + "' lacks '.pseudo' extension"
		};
	}

	/* If the target output flag is not valid */
	if (!is_valid_output_target(get_target_output_flag()))
	{
		/* Error out */
		CLIArgsError
		{
			"target output flag '" + get_target_output_flag() + "' unrecognized"
		};
	}
}

void CLIArgs::file_to_source(const std::string& path)
{
	/* Creates new ifstream object for the file */
	std::ifstream file{};

	/* Creates a handle to the file from the path */
	file.open(path);

	/* If the file did not open properly */
	if (!file.is_open())
	{
		/* Error out */
		CLIArgsError
		{
			"file '" + get_source_path() + "' could not be opened"
		};
	}

	/* Read the source contents into the source member */
	m_source = {std::istreambuf_iterator<char>(file),
	            std::istreambuf_iterator<char>()};

	/* Close the file handle */
	file.close();
}

[[nodiscard]] bool CLIArgs::is_valid_output_target(const std::string_view output_target)
{
	return output_target == "exe"; /* Return whether the output target is valid */
}
