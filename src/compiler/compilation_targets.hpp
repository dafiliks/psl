/* compiler/compilation_targets.hpp by David Filiks */
/* The compilation targets header for the PsL compiler */

#ifndef COMPILATION_TARGETS_HPP
#define COMPILATION_TARGETS_HPP

#include <string>

/* Enum class of all the possible output format targets */
enum class OutputTarget
{
	EXE, /* Represents a desired output target of an executable/binary */
	ASM, /* Represents a desired output target of assembly langauge */
	OBJ, /* Represents a desired output target of an object file */
};

/* Base compilation target struct, can represent any compilation target */
struct CompilationTarget
{
	/* Functions */

	/* Constructs a CompilationTarget object */
	/* Param: const OutputTarget& - the output target */
	CompilationTarget(const OutputTarget& target);

	/* Compile the ".cpp" file into the output target */
	/* Each target will override this function and implement it's own execution behavior */
	/* Param: const std::string_view - the name of the ".cpp" file */
	virtual void compile(const std::string_view cpp_file) = 0;

	/* Variables */

	OutputTarget m_target{}; /* The desired output target */
};

/* The GNU Compiler Collection target, inherits from CompilationTarget as it is a compilation target */
struct GCCTarget : public CompilationTarget
{
	/* Constructs a GCCTarget object */
	/* Param: const OutputTarget& - the output target */
	GCCTarget(const OutputTarget& target);

	/* Compile the ".cpp" file into the output target using GCC */
	/* Param: const std::string_view - the name of the ".cpp" file */
	void compile(const std::string_view cpp_file) override;
};

#endif
