/* main.cpp by David Filiks */
/* The entry point implementation for the PsL compiler */

#include "compiler/compiler.hpp"
#include "utils/cliargs.hpp"

/* Entry point function */
/* Param: int - argument count */
/* Param: char** - argument vector */
int main(int argc, char** argv)
{
	/* Try to run the following code */
	try
	{
		/* Construct a CLIArgs object, passing argc, and argv */
		CLIArgs args{argc, argv};

		/* Handle CLI arguments */
		args.handle();

		/* Construct a Compiler object */
		Compiler compiler{args};

		/* Fully compile the source into desired output */
		compiler.compile();
	}

	/* If an Error is thrown */
	catch (const Error& e) 
	{
		/* Handle any compilation process error */
		std::cerr << e.what() << std::endl;

		/* Return EXIT_FAILURE to signify failure */
		return EXIT_FAILURE;
	}

	/* If a standard exception is thrown */
	catch (const std::exception& e)
	{
		/* Handle any standard exception */
		std::cerr << "standard exception: " << e.what() << std::endl;

		/* Return EXIT_FAILURE to signify failure */
		return EXIT_FAILURE;
	}

	/* If any other exception is thrown */
	catch (...) 
	{
		/* Handle all other (unknown) exceptions */
		std::cerr << "unknown exception thrown" << std::endl;

		/* Return EXIT_FAILURE to signify failure */
		return EXIT_FAILURE;
	}

	/* Return EXIT_SUCCESS to signify success */
	return EXIT_SUCCESS;
}
