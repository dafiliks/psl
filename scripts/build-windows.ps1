$SRC = @("..\src\main.cpp",
         "..\src\frontend\lexer.cpp",
         "..\src\frontend\parser.cpp",
         "..\src\backend\gen.cpp",
         "..\src\utils\error.cpp",
         "..\src\utils\error_types.cpp",
         "..\src\utils\cliargs.cpp",
         "..\src\compiler\compiler.cpp",
         "..\src\compiler\compilation_targets.cpp")

$OUT = "psl.exe"

if (Get-Command g++ -ErrorAction SilentlyContinue)
{
	$COMPILER = "g++"
}
elseif (Get-Command cl -ErrorAction SilentlyContinue)
{
	$COMPILER = "cl"
}
elseif (Get-Command clang++ -ErrorAction SilentlyContinue)
{
	$COMPILER = "clang++"
}
else
{
	exit 1
}

$CXXFLAGS = "/std:c++20 /Zi"

if ($COMPILER -eq "cl")
{
	cl /EHsc $CXXFLAGS $SRC /Fe:$OUT
}
else
{
	& $COMPILER $SRC -std:c++20 -g -o $OUT
}
