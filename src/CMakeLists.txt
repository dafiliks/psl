add_executable(psl "main.cpp" "frontend/lexer.cpp" "frontend/parser.cpp"
                   "backend/gen.cpp" "utils/error.cpp" "utils/error_types.cpp"
                   "utils/cliargs.cpp" "compiler/compiler.cpp" "compiler/compiler_stage.cpp"
                   "compiler/compilation_targets.cpp" "utils/stack.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET psl PROPERTY CXX_STANDARD 23)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_BUILD_TYPE Debug)
