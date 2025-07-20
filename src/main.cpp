#include <iostream>
#include <cstring>
#include <cstdlib>
#include <filesystem>

#include "frontend/lexer.hpp"
#include "frontend/parser.hpp"
#include "backend/gen.hpp"

#include "compiler/compiler.hpp"
#include "utils/cmdargs.hpp"

int main(int argc, char** argv)
{
	CmdArgs args{argc, argv};
	args.handle();

	Compiler compiler{args};
	compiler.compile();

	return 0;
}
