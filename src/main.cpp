/* main.cpp by David Filiks */
/* The entry point implementation for the PsL compiler */

#include "compiler/compiler.hpp"
#include "utils/cliargs.hpp"

/* Entry point function */
/* Param: int - argument count */
/* Param: char** - argument vector */
int main(int argc, char** argv)
{
	/* Construct a CLIArgs object, passing argc, and argv */
	CLIArgs args{argc, argv};

	/* Handle CLI arguments */
	args.handle();

	/* Construct a Compiler object */
	Compiler compiler{args};

	/* Fully compile the source into desired output */
	compiler.compile();

	/* Return zero to signify success, not needed but I want to be explicit */
	return 0;
}
