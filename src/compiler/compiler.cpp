#include "compiler.hpp"
#include "../utils/cliargs.hpp"
#include "../frontend/parser.hpp"
#include "../compiler/compilation_targets.hpp"
#include "../frontend/lexer.hpp"
#include "../backend/gen.hpp"
#include "../utils/error_types.hpp"

Compiler::Compiler(const CLIArgs& args)
: m_args(args), m_lexer(m_args), m_parser(m_lexer), m_gen(m_parser) /* Initializes all member variables */ {}

void Compiler::compile()
{
	/* Compile source file to a C++20 compliant ".cpp" file */
	compile_to_cpp();

	/* Compile the ".cpp" file into the desired output target */
	compile_to_output_target(m_args.get_source_path());
}

void Compiler::compile_to_cpp()
{
	/* Create a vector of CompilationStage references, each of which represents a stage in compilation */
	std::vector<std::reference_wrapper<CompilationStage>> compilation_stages
	{
		std::ref(m_lexer), /* Reference to lexer object */
		std::ref(m_parser), /* Reference to parser object */
		std::ref(m_gen) /* Reference to generator object */
	};

	/* Loop through the compilation stages */
	for (CompilationStage& compilation_stage : compilation_stages)
	{
		/* Execute each compilation stage sequentially */
		compilation_stage.execute();
	}
}

void Compiler::compile_to_output_target(const std::string_view cpp_file)
{
	/* Declare a smart pointer to a CompilationTarget */
	std::unique_ptr<CompilationTarget> target{};

	/* If the target output flag is "-exe" */
	if (m_args.get_target_output_flag() == "-exe")
	{
/* If the user is using the G++ compiler */
#if defined(__GNUC__) && defined(__cplusplus)
		/* Set the target compiler to GCC (G++) and the output target to EXE */
		target = std::make_unique<GCCTarget>(OutputTarget::EXE);
#endif
	}

	/* If the target was set */
	if (target)
	{
		/* Compile into the output target using the desired compiler */
		target->compile(cpp_file);
	}

	/* If the target was not set */
	else
	{
		/* Error out */
		CompileError
		{
			"no valid compilation target found for flag " + m_args.get_target_output_flag()
		};
	}
}
