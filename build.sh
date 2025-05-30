# only linux compatible for now
g++ -g -fsanitize=address -o psl main.cpp lexer.cpp parser.cpp gen.cpp
