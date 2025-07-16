add_executable(psl "main.cpp" "lexer.cpp" "parser.cpp" "gen.cpp" "error.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET psl PROPERTY CXX_STANDARD 20)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_BUILD_TYPE Debug)
