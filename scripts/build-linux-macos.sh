SRC=("../src/main.cpp" \
     "../src/frontend/lexer.cpp" \
     "../src/frontend/parser.cpp" \
     "../src/backend/gen.cpp" \
     "../src/utils/error.cpp" \
     "../src/utils/error_types.cpp" \
     "../src/utils/cliargs.cpp" \
     "../src/compiler/compiler.cpp" \
     "../src/compiler/compilation_targets.cpp")

OUT="psl"

if command -v g++ >/dev/null 2>&1; then
	COMPILER="g++"
elif command -v clang++ >/dev/null 2>&1; then
	COMPILER="clang++"
else
	exit 1
fi

CXXFLAGS="-std=c++20 -g"

$COMPILER "${SRC[@]}" $CXXFLAGS -o $OUT
