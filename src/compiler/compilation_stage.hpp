/* compiler/compilation_stage.hpp by David Filiks */
/* The compilation stage header for the PsL compiler */

#ifndef COMPILATION_STAGE_HPP
#define COMPILATION_STAGE_HPP

/* Struct representing a stage in the compilation cycle */
struct CompilationStage {
	/* Executes the particular compilation stage */
	/* Each stage will override this function and implement it's own execution behavior */
	virtual void execute() = 0;
};

#endif
