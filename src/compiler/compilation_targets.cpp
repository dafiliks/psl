/* compiler/compilations_targets.cpp by David Filiks */
/* The compilation targets implementation for the PsL compiler */

#include "compilation_targets.hpp"

CompilationTarget::CompilationTarget(const OutputTarget& target)
: m_target(target) /* Initialize output target */ {}

GCCTarget::GCCTarget(const OutputTarget& target)
: CompilationTarget(target) /* Initialize CompilationTarget object */ {}

void GCCTarget::compile(const std::string_view cpp_file)
{
	/* Deduce output file name, by removing the ".cpp" from the file name */
	std::filesystem::path output_file{cpp_file};
	output_file.replace_extension("");

	/* Variable that holds the final command that will be executed */
	std::string command_str{};

	/* Switch through all the possible output targets */
	switch (m_target)
	{
		/* If the output target is EXE */
		case (OutputTarget::EXE):
			/* CLI command to compile ".cpp" file to EXE with GCC */
			command_str = "g++ " + output_file.string() + ".cpp -std=c++20 -o " + output_file.string() + " && ./" + output_file.string();

			/* Break from switch case */
			break;

		/* If the output target is ASM */
		case (OutputTarget::ASM):
			/* CLI command to compile ".cpp" file to ASM with GCC */
			command_str = "g++ -S " + output_file.string() + ".cpp -std=c++20 -o " + output_file.string() + ".s";

			/* Break from switch case */
			break;

		/* If the output target is OBJ */
		case (OutputTarget::OBJ):
			/* CLI command to compile ".cpp" file to OBJ with GCC */
			command_str = "g++ -c " + output_file.string() + ".cpp -std=c++20 -o " + output_file.string() + ".o";

			/* Break from switch case */
			break;

		/* Should never reach this point, but in case of something weird like a bit flip */
		default:
			/* Error out */
			CompileError
			{
				"output target is unrecognized - re-executing the program might help"
			};

			/* Break from switch case */
			break;

	}

	/* Execute the final command with the user's default shell */
	std::system(command_str.c_str());
}