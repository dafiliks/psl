/* utils/cliargs.cpp by David Filiks */
/* The cli args implementation for the PsL compiler */

#include "../utils/cliargs.hpp"

CLIArgs::CLIArgs(int argc, char **argv)
/* Initialize argc and argv members */
: m_argc(argc),
  m_argv(argv) {}

void CLIArgs::handle()
{
	/* Validate the CLI arguments */
	validate_args();

	/* Store the source file contents as a string */
	file_to_source(get_source_path());
}

[[nodiscard]] int CLIArgs::get_argc() const
{
	/* Return argc (the argument count) */
	return m_argc;
}

[[nodiscard]] char** CLIArgs::get_argv() const
{
	/* Return argv (the arguemnt strings) */
	return m_argv;
}

[[nodiscard]] const std::string& CLIArgs::get_source() const
{
	/* Return the source contents */
	return m_source;
}

[[nodiscard]] std::string CLIArgs::get_source_path() const
{
	/* Return string copy of the source path */
	return std::string{m_argv[1]};
}

[[nodiscard]] std::string CLIArgs::get_target_output_flag() const
{
	/* Return string copy of the target output flag */
	return std::string{m_argv[2]};
}

void CLIArgs::validate_args()
{
	/* If the argument count is not equal to two */
	if (m_argc != 3)
	{
		/* Throw cli args error */
		throw CLIArgsError
		{
			"wrong number of arguments provided"
		};
	}

	/* If the file does not exist */
	if (!std::filesystem::exists(get_source_path()))
	{
		/* Throw cli args error */
		throw CLIArgsError
		{
			"file '" + get_source_path() + "' does not exist"
		};

	}

	/* If the file extension is not ".pseudo" */
	if (std::filesystem::path(get_source_path()).extension() != ".pseudo")
	{
		/* Throw cli args error */
		throw CLIArgsError
		{
			"file '" + get_source_path() + "' lacks '.pseudo' extension"
		};
	}

	/* If the target output flag is not valid */
	if (!is_valid_output_target(get_target_output_flag()))
	{
		/* Throw cli args error */
		throw CLIArgsError
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
		/* Throw cli args error */
		throw CLIArgsError
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
	/* Return whether the output target is valid */
	return output_target == "-exe" ||
	       output_target == "-asm" ||
	       output_target == "-obj";
}
